
#include "PrivateInclude.hpp"

#include "Painter_PaintDevice.hpp"

#include <memory>
#include <utility>

namespace e00 {
struct Sprite::Image {
  std::unique_ptr<Bitmap> bitmap;
  std::chrono::milliseconds duration{};
};

/******************************************************************************
 *
 * Sprite
 * 
 *****************************************************************************/

std::unique_ptr<Sprite> Sprite::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette) {
  return std::unique_ptr<Sprite>(new Sprite(size, bit_depth, numColorsInPalette));
}

Sprite::~Sprite() = default;

size_t Sprite::SizeUsage() {
  size_t total = 0;
  for (const auto &image: _images) {
    if (image->bitmap)
      total += image->bitmap->SizeUsage();
  }
  return sizeof(*this) + _images.size() * sizeof(Image) + total;
}

std::unique_ptr<Sprite> Sprite::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette) {
  return std::unique_ptr<Sprite>(new Sprite(size, bit_depth, std::move(palette)));
}

Sprite::Sprite(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette)
    : DrawableResource(size, bit_depth),
      _palette(std::move(palette)),
      _current_time(0),
      _total_time(0),
      _loops(true),
      _current_image(_images.end()) {
}

Sprite::Sprite(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette)
    : DrawableResource(size, bit_depth),
      _palette(numColorsInPalette),
      _current_time(0),
      _total_time(0),
      _loops(true),
      _current_image(_images.end()) {
}

void Sprite::SetCurrentTime(const std::chrono::milliseconds time) {
  _current_time = time;

  // Clamp or wrap the current animation time into the valid animation range.
  // Empty animations need special handling because _total_time may be zero.
  if (_current_time > _total_time) {
    if (_loops) {
      while (_current_time > _total_time) {
        _current_time = _current_time - _total_time;
      }
    } else {
      _current_time = _total_time;
    }
  }

  // Walk the frame list until the frame containing the current animation time is found.
  decltype(_current_time) current_time = _current_time;
  for (auto it = _images.begin(); it != _images.end() && *it != nullptr; ++it) {
    _current_image = it;
    if (current_time <= (*it)->duration) {
      break;
    }
    current_time -= (*it)->duration;
  }
}

void Sprite::SetImageIndex(size_t index) {
  if (_loops && index >= _images.size()) {
    index %= _images.size();
  }

  if (index < _images.size()) {
    _current_image = _images.begin() + index;
  }
}

std::unique_ptr<Painter> Sprite::BeginDraw() {
  if (const auto *image = GetCurrentImage()) {
    return std::make_unique<SoftwarePainter>(*image->bitmap);
  }

  return nullptr;
}

bool Sprite::HasTransparencyMask() const {
  if (const auto *image = GetCurrentImage()) {
    return image->bitmap->HasTransparencyMask();
  }
  return false;
}

void Sprite::ReadLineInto(
    BitmapSizeType line,
    BitmapSizeType startX, BitmapSizeType endX,
    const TargetInformation &targetInformation,
    const std::span<uint8_t> &targetBuffer) const {
  if (const auto *image = GetCurrentImage()) {
    image->bitmap->ReadLineInto(line, startX, endX, targetInformation, targetBuffer);
  }
}
void Sprite::ReadTransparencyMaskLineInto(BitmapSizeType line, BitmapSizeType startX, BitmapSizeType endX, const std::span<uint8_t> &targetBuffer) const {
  if (HasTransparencyMask()) {
  }
}

error_code Sprite::AddFrame(ResourcePtrT<Bitmap> data, std::chrono::milliseconds duration) {
  if (!data) {
    return make_error_code(errc::invalid_argument);
  }

  // The frame must match the sprite format so all frames can be rendered through
  // the same DrawableSurface interface.
  if (data->GetBitDepth() != GetBitDepth()) {
    return make_error_code(errc::invalid_argument);
  }

  // Find the first null frame
  auto it = std::ranges::find_if(_images, [](const auto &frame) { return !frame; });
  if (it == _images.end()) {
    return e00::make_error_code(errc::not_enough_memory);
  }

  auto frame = std::make_unique<Image>();
  frame->bitmap = std::make_unique<Bitmap>(Size(), GetBitDepth());
  frame->duration = duration;

  // Preserve the source bitmap's palette so per-pixel palette lookups on the
  // copied frame remain valid (indices decoded into the source data must map
  // into the same set of colors on the frame).
  frame->bitmap->SetPalette(data->_palette);

  // Copy data
  const auto copyHeight = std::min(Size().y, data->Size().y);
  for (BitmapSizeType y = 0; y < copyHeight; ++y) {
    auto srcLine = data->helper.GetLineData(std::span(data->_data), y);
    auto dstLine = frame->bitmap->helper.GetLineData(std::span(frame->bitmap->_data), y);
    std::memcpy(dstLine.data(), srcLine.data(), std::min(srcLine.size(), dstLine.size()));
  }

  // Copy/allocate the transparency mask from the source bitmap, if present
  if (data->HasTransparencyMask()) {
    frame->bitmap->EnableTransparencyMask();
    const auto copyWidth = std::min(Size().x, data->Size().x);
    for (BitmapSizeType y = 0; y < copyHeight; ++y) {
      for (BitmapSizeType x = 0; x < copyWidth; ++x) {
        frame->bitmap->SetMaskPixel(x, y, data->IsOpaqueAt(x, y));
      }
    }
  }

  *it = std::move(frame);
  _total_time += duration;

  // Select the first frame automatically so the sprite becomes drawable as soon
  // as at least one frame has been added.
  if (_current_image == _images.end()) {
    _current_image = it;
  }

  return {};
}

error_code Sprite::AddFrame(std::unique_ptr<Bitmap> &&data, std::chrono::milliseconds duration) {
  return AddFrame(ResourceManager::GlobalResourceManager().TakeOwnership(std::move(data)), duration);
}

void Sprite::Paint(Painter &painter, const BitmapPosition &position) {
  painter.BlitSurface(*this, {{0, 0}, Size()}, Vec2D<BitmapSizeType>{position.x, position.y});
}
}// namespace e00
