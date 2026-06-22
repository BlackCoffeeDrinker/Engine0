#include "BitmapData.hpp"
#include "Painter_PaintDevice.hpp"
#include <Engine/Platform/ResourceManager.hpp>


#include <memory>
#include <utility>

#include "PrivateInclude.hpp"

namespace e00 {
struct Sprite::Image {
  std::unique_ptr<impl::BitmapData> bitmap;
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
    return std::make_unique<SoftwarePainter>(Size(), GetBitDepth(), _palette, *image->bitmap);
  }

  return nullptr;
}

void Sprite::ReadLineInto(
    BitmapSizeType line,
    BitmapSizeType startX, BitmapSizeType endX,
    const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const {
  if (const auto *image = GetCurrentImage()) {
    image->bitmap->ReadLineInto(line, startX, endX, targetInformation, GetBitDepth(), _palette, targetBuffer);
  }
}

std::error_code Sprite::AddFrame(ResourcePtrT<Bitmap> data, std::chrono::milliseconds duration) {
  if (!data) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // The frame must match the sprite format so all frames can be rendered through
  // the same DrawableSurface interface.
  if (data->GetBitDepth() != GetBitDepth()) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Find the first null frame
  auto it = std::ranges::find_if(_images, [](const auto &frame) { return !frame; });
  if (it == _images.end()) {
    return std::make_error_code(std::errc::not_enough_memory);
  }

  auto frame = std::make_unique<Image>();
  frame->bitmap = std::make_unique<impl::BitmapData>(Size(), GetBitDepth());
  frame->duration = duration;

  // Copy data
  const auto copyHeight = std::min(Size().y, data->Size().y);
  for (BitmapSizeType y = 0; y < copyHeight; ++y) {
    auto srcLine = data->GetLineData(y);
    auto dstLine = frame->bitmap->GetLineSpan(y);
    std::memcpy(dstLine.data(), srcLine.data(), std::min(srcLine.size(), dstLine.size()));
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

std::error_code Sprite::AddFrame(std::unique_ptr<Bitmap> &&data, std::chrono::milliseconds duration) {
  return AddFrame(ResourceManager::GlobalResourceManager().TakeOwnership(std::move(data)), duration);
}
}// namespace e00
