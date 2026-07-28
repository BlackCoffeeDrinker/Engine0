#include "Painter_PaintDevice.hpp"
#include "PrivateInclude.hpp"

#include <Engine/DefaultBitmapHelpers.hpp>

#include <span>
#include <utility>

namespace e00 {
namespace detail {
SoftwareBitmapHelper::SoftwareBitmapHelper() : bit_depth(DrawableSurface::BitDepth::DEPTH_INVALID) {
}

SoftwareBitmapHelper::SoftwareBitmapHelper(DrawableSurface::BitDepth bit_depth, BitmapSizeType width, BitmapSizeType height)
    : bit_depth(bit_depth),
      bytes_per_line(helpers::GetBitmapBufferBytesPerLine(bit_depth, width)),
      valid_data_per_line(helpers::GetBitmapValidBytesPerLine(bit_depth, width)),
      buffer_size(helpers::GetBufferSizeForBitmapSize(bit_depth, width, height)) {
  assert(bit_depth != DrawableSurface::BitDepth::DEPTH_INVALID);

  // make defaults for 16 & 32bits
  if (bit_depth == DrawableSurface::BitDepth::DEPTH_16) {
    shift = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_16>::DefaultShift;
    mask = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_16>::DefaultMask;
  } else if (bit_depth == DrawableSurface::BitDepth::DEPTH_32) {
    shift = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_32>::DefaultShift;
    mask = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_32>::DefaultMask;
  }
}
}// namespace detail

std::unique_ptr<Bitmap> Bitmap::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette) {
  auto ret = std::make_unique<Bitmap>(size, bit_depth);
  ret->SetPalette(palette);
  return ret;
}

std::unique_ptr<Bitmap> Bitmap::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette) {
  auto bitmap = std::make_unique<Bitmap>(size, bit_depth);
  bitmap->SetPalette(FixedPalette(numColorsInPalette));
  return bitmap;
}

Bitmap::Bitmap(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, MemoryAlignment lineAlignment)
    : DrawableResource(size, bit_depth),
      helper(bit_depth, size.x, size.y) {
  _data.resize(helper.buffer_size);
}


Bitmap::~Bitmap() = default;

Color Bitmap::ReadColorAt(BitmapSizeType x, BitmapSizeType y) const {
  if (const auto srcLine = helper.GetLineData(std::span(_data), y); !srcLine.empty()) {
    switch (GetBitDepth()) {
      case BitDepth::DEPTH_1: return helpers::BitmapDepth1::ReadColor(srcLine, x) ? _palette[1] : _palette[0];
      case BitDepth::DEPTH_8: return _palette[helpers::BitmapDepth8::ReadColor(srcLine, x)];
      case BitDepth::DEPTH_16: return helpers::BitmapDepth16::ReadColor(srcLine, x, GetShift(), GetMask());
      case BitDepth::DEPTH_32: return helpers::BitmapDepth32::ReadColor(srcLine, x, GetShift(), GetMask());
      case BitDepth::DEPTH_8_NO_PALETTE: std::abort();
      case BitDepth::DEPTH_INVALID: std::abort();
    }
  }

  return {};
}

std::unique_ptr<Painter> Bitmap::BeginDraw() {
  return std::make_unique<SoftwarePainter>(*this);
}

size_t Bitmap::MaskBytesPerLine() const {
  return helpers::BitmapDepth1::BufferBytesPerLine(Size().x);
}

void Bitmap::EnableTransparencyMask() {
  if (!_mask.empty()) {
    return;
  }

  // Default every pixel to opaque (all bits set)
  _mask.assign(MaskBytesPerLine() * Size().y, 0xFFu);
}

void Bitmap::SetMaskPixel(BitmapSizeType x, BitmapSizeType y, bool opaque) {
  if (_mask.empty() || y >= Size().y || x >= Size().x) {
    return;
  }

  const auto bytesPerLine = MaskBytesPerLine();
  const auto line = std::span(_mask).subspan(y * bytesPerLine, bytesPerLine);
  helpers::BitmapDepth1::WriteColor(line, x, opaque);
}

bool Bitmap::IsOpaqueAt(BitmapSizeType x, BitmapSizeType y) const {
  if (_mask.empty()) {
    return true;
  }

  if (y >= Size().y || x >= Size().x) {
    return true;
  }

  const auto bytesPerLine = MaskBytesPerLine();
  const auto line = std::span(_mask).subspan(y * bytesPerLine, bytesPerLine);
  return helpers::BitmapDepth1::ReadColor(line, x);
}

void Bitmap::ReadLineInto(BitmapSizeType line,
                          BitmapSizeType startX,
                          BitmapSizeType endX,
                          const TargetInformation &targetInformation,
                          const std::span<uint8_t> &targetBuffer) const {
  const auto dstDepth = targetInformation.bit_depth;
  const auto srcDepth = GetBitDepth();
  const BitmapSizeType width = endX - startX;
  const auto *targetPalette = targetInformation.palette;

  // Make sure we have a target palette if we need it
  if (helpers::needsPalette(dstDepth) && targetPalette == nullptr) {
    GetDefaultLogger().Error(
        source_location::current(),
        "Invalid target: 8-bit destination with null palette");
    std::abort();
    return;
  }

#ifndef NDEBUG
  assert(!targetBuffer.empty());
  assert(targetBuffer.size() >= helpers::GetBitmapValidBytesPerLine(targetInformation.bit_depth, width));
#endif

  const auto both8Bit = helpers::is8Bit(srcDepth) && helpers::is8Bit(dstDepth);
  const auto eitherSideDontNeedPalette = srcDepth == BitDepth::DEPTH_8_NO_PALETTE || dstDepth == BitDepth::DEPTH_8_NO_PALETTE;

  // Optimized path for 8-bit to 8-bit matching
  // If either is NO_PALETTE, we don't care about palette matching, it's just a raw copy
  if (both8Bit && (eitherSideDontNeedPalette || (targetPalette && _palette.isSamePalette(*targetPalette)))) {
    const auto srcLine = helper.GetLineData(std::span(_data), line);
    memcpy(
        targetBuffer.data(),
        srcLine.data() + startX,
        helpers::BitmapDepth8::ValidBytesPerLine(width));

    return;
  }

  // Optimized path for 32-bit to 32-bit matching (assuming standard layout)
  if (srcDepth == BitDepth::DEPTH_32 && dstDepth == BitDepth::DEPTH_32 &&
      targetInformation.shift == GetShift() && targetInformation.mask == GetMask()) {
    const auto srcLine = helper.GetLineData(std::span(_data), line);
    memcpy(
        targetBuffer.data(),
        srcLine.data() + startX * 4,
        helpers::BitmapDepth32::ValidBytesPerLine(width));
    return;
  }

  // Might be able to conver to 8 bit no palette: if a reference one was provided
  if (dstDepth == BitDepth::DEPTH_8_NO_PALETTE && targetPalette == nullptr) {
    GetDefaultLogger().Error(
        source_location::current(),
        "Cannot convert to DEPTH_8_NO_PALETTE without a reference palette");
    std::abort();
  }

  // But we can't do anything if the _source_ is 8 bit no palette
  if (srcDepth == BitDepth::DEPTH_8_NO_PALETTE) {
    GetDefaultLogger().Error(
        source_location::current(),
        "Cannot convert from DEPTH_8_NO_PALETTE");
    std::abort();
  }

  // Generic implementation
  for (BitmapSizeType x = 0; x < width; ++x) {
    const auto srcX = startX + x;
    using Depth = BitDepth;

    // Extract color from source
    Color c = ReadColorAt(srcX, line);

    // Write color to targetBuffer based on targetInformation
    using namespace e00::helpers;
    switch (dstDepth) {
      case Depth::DEPTH_1: BitmapDepth1::WriteColor(targetBuffer, x, targetPalette->findClosestColorIndex(c)); break;
      case Depth::DEPTH_8: BitmapDepth8::WriteColor(targetBuffer, x, targetPalette->findClosestColorIndex(c)); break;
      case Depth::DEPTH_8_NO_PALETTE: BitmapDepth8::WriteColor(targetBuffer, x, targetPalette->findClosestColorIndex(c)); break;
      case Depth::DEPTH_16: BitmapDepth16::WriteColor(targetBuffer, x, c, targetInformation.shift, targetInformation.mask); break;
      case Depth::DEPTH_32: BitmapDepth32::WriteColor(targetBuffer, x, c, targetInformation.shift, targetInformation.mask); break;
      case Depth::DEPTH_INVALID: std::abort();
    }
  }
}

void Bitmap::ReadTransparencyMaskLineInto(BitmapSizeType line, BitmapSizeType startX, BitmapSizeType endX, const std::span<uint8_t> &targetBuffer) const {
  if (HasTransparencyMask()) {
    // TODO
  }
}


}// namespace e00
