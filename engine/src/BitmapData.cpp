
#include "BitmapData.hpp"

#include <bit>
#include <cstdint>

namespace e00::impl {
BitmapData::BitmapData(const Vec2D<uint16_t> &size, DrawableSurface::BitDepth bit_depth, [[maybe_unused]] MemoryAlignment lineAlignment)
    : bytes_per_line(),
      valid_data_per_line(0) {
  // make defaults for 16 & 32bits
  if (bit_depth == DrawableSurface::BitDepth::DEPTH_16) {
    shift = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_16>::DefaultShift;
    mask = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_16>::DefaultMask;
  } else if (bit_depth == DrawableSurface::BitDepth::DEPTH_32) {
    shift = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_32>::DefaultShift;
    mask = helpers::BitmapHelper_t<DrawableSurface::BitDepth::DEPTH_32>::DefaultMask;
  }

  // Compute image size
  const auto depth = DepthEnumToBits(bit_depth);
  bytes_per_line = ((size.x * depth + 31u) / 32u) * 4u;// bytes per line
  data.resize(bytes_per_line * size.y);

  switch (bit_depth) {
    using enum DrawableSurface::BitDepth;
    case DEPTH_1:
      valid_data_per_line = helpers::BitmapHelper_t<DEPTH_1>::ValidBytesPerLine(size.x);
      break;
    case DEPTH_8:
    case DEPTH_8_NO_PALETTE:
      valid_data_per_line = helpers::BitmapHelper_t<DEPTH_8>::ValidBytesPerLine(size.x);
      break;
    case DEPTH_16:
      valid_data_per_line = helpers::BitmapHelper_t<DEPTH_16>::ValidBytesPerLine(size.x);
      break;
    case DEPTH_32:
      valid_data_per_line = helpers::BitmapHelper_t<DEPTH_32>::ValidBytesPerLine(size.x);
      break;
    case DEPTH_INVALID:
      break;
  }
}

uint8_t BitmapData::ReadColor8(BitmapSizeType x, BitmapSizeType y) const {
  if (const auto srcLine = GetLineSpan(y); !srcLine.empty()) {
    return helpers::BitmapDepth8::ReadColor(srcLine, x);
  }
  return 0;
}

bool BitmapData::ReadColor1(BitmapSizeType x, BitmapSizeType y) const {
  if (const auto srcLine = GetLineSpan(y); !srcLine.empty()) {
    return helpers::BitmapDepth1::ReadColor(srcLine, x);
  }

  return false;
}

void BitmapData::WriteColor8(BitmapSizeType x, BitmapSizeType y, uint8_t color) {
  if (auto dstLine = GetLineSpan(y); !dstLine.empty()) {
    helpers::BitmapDepth8::WriteColor(dstLine, x, color);
  }
}

Color BitmapData::ReadColor16(BitmapSizeType x, BitmapSizeType y) const {
  if (const auto srcLine = GetLineSpan(y); !srcLine.empty()) {
    return helpers::BitmapDepth16::ReadColor(srcLine, x, shift, mask);
  }
  return {};
}

Color BitmapData::ReadColor32(BitmapSizeType x, BitmapSizeType y) const {
  if (const auto srcLine = GetLineSpan(y); !srcLine.empty()) {
    return helpers::BitmapDepth32::ReadColor(srcLine, x, shift, mask);
  }
  return {};
}

void BitmapData::WriteColor16(BitmapSizeType x, BitmapSizeType y, const Color &color) {
  if (auto dstLine = GetLineSpan(y); !dstLine.empty()) {
    helpers::BitmapDepth16::WriteColor(dstLine, x, color, shift, mask);
  }
}

void BitmapData::WriteColor32(BitmapSizeType x, BitmapSizeType y, const Color &color) {
  if (auto dstLine = GetLineSpan(y); !dstLine.empty()) {
    helpers::BitmapDepth32::WriteColor(dstLine, x, color, shift, mask);
  }
}

void BitmapData::WriteColor1(BitmapSizeType x, BitmapSizeType y, bool color) {
  if (auto dstLine = GetLineSpan(y); !dstLine.empty()) {
    helpers::BitmapDepth1::WriteColor(dstLine, x, color);
  }
}

void BitmapData::ReadLineInto(
    BitmapSizeType line,
    BitmapSizeType startX, BitmapSizeType endX,
    const DrawableSurface::TargetInformation &targetInformation,
    DrawableSurface::BitDepth srcDepth, const FixedPalette &sourcePalette,
    std::span<uint8_t> targetBuffer) const {
  const auto dstDepth = targetInformation.bit_depth;
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

  const auto both8Bit = helpers::is8Bit(srcDepth) && helpers::is8Bit(dstDepth);
  const auto eitherSideDontNeedPalette = srcDepth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE || dstDepth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE;

  // Optimized path for 8-bit to 8-bit matching
  // If either is NO_PALETTE, we don't care about palette matching, it's just a raw copy
  if (both8Bit && (eitherSideDontNeedPalette || (targetPalette && sourcePalette.isSamePalette(*targetPalette)))) {
    const auto srcLine = GetLineSpan(line);
    memcpy(
        targetBuffer.data(),
        srcLine.data() + startX,
        helpers::BitmapDepth8::ValidBytesPerLine(width));

    return;
  }

  // Optimized path for 32-bit to 32-bit matching (assuming standard layout)
  if (srcDepth == DrawableSurface::BitDepth::DEPTH_32 && dstDepth == DrawableSurface::BitDepth::DEPTH_32 &&
      targetInformation.shift == GetShift() && targetInformation.mask == GetMask()) {
    const auto srcLine = GetLineSpan(line);
    memcpy(
        targetBuffer.data(),
        srcLine.data() + startX * 4,
        helpers::BitmapDepth32::ValidBytesPerLine(width));
    return;
  }

  // Might be able to conver to 8 bit no palette: if a reference one was provided
  if (dstDepth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE && targetPalette == nullptr) {
    GetDefaultLogger().Error(
        source_location::current(),
        "Cannot convert to DEPTH_8_NO_PALETTE without a reference palette");
    std::abort();
  }

  // But we can't do anything if the _source_ is 8 bit no palette
  if (srcDepth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE) {
    GetDefaultLogger().Error(
        source_location::current(),
        "Cannot convert from DEPTH_8_NO_PALETTE");
    std::abort();
  }

  // Generic implementation
  for (BitmapSizeType x = 0; x < width; ++x) {
    const auto srcX = startX + x;
    using Depth = DrawableSurface::BitDepth;

    // Extract color from source
    Color c;
    switch (srcDepth) {
      case Depth::DEPTH_1: c = ReadColor1(srcX, line) ? sourcePalette[1] : sourcePalette[0]; break;
      case Depth::DEPTH_8: c = sourcePalette[ReadColor8(srcX, line)]; break;
      case Depth::DEPTH_16: c = ReadColor16(srcX, line); break;
      case Depth::DEPTH_32: c = ReadColor32(srcX, line); break;
      case Depth::DEPTH_8_NO_PALETTE: std::abort();
      case Depth::DEPTH_INVALID: std::abort();
    }

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


}// namespace e00::impl
