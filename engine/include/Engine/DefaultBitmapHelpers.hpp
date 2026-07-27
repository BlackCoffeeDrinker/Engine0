
#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <span>

#include <Engine/Math/Color.hpp>
#include <Engine/Platform/DrawableSurface.hpp>

namespace e00::helpers {

template<typename T>
T load_pixel(std::span<const uint8_t> lineData, std::size_t x) {
  T v{};
  std::memcpy(&v, lineData.data() + x * sizeof(T), sizeof(T));
  return v;
}

template<typename T>
void store_pixel(const std::span<uint8_t> &lineData, std::size_t x, T v) {
  std::memcpy(lineData.data() + x * sizeof(T), &v, sizeof(T));
}

inline bool is8Bit(DrawableSurface::BitDepth depth) {
  return depth == DrawableSurface::BitDepth::DEPTH_8 || depth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE;
}

inline bool needsPalette(DrawableSurface::BitDepth depth) {
  return depth == DrawableSurface::BitDepth::DEPTH_8 || depth == DrawableSurface::BitDepth::DEPTH_1;
}

struct BitmapDepth1 {
  static constexpr uint8_t BitsPerPixel = 1;
  using Type = uint8_t;

  static constexpr DrawableSurface::RGBInfo DefaultShift = {0, 0, 0};
  static constexpr DrawableSurface::RGBInfo DefaultMask = {0, 0, 0};

  static constexpr size_t BufferBytesPerLine(size_t width) { return ((width * BitsPerPixel + 31u) / 32u) * 4u; }
  static constexpr size_t ValidBytesPerLine(size_t width) { return (width + 7) / 8; }

  static bool ReadColor(std::span<const uint8_t> lineData, BitmapSizeType x) {
    const auto byteIndex = x / 8;
    if (byteIndex >= lineData.size())
      return false;

    const auto bitMask = static_cast<uint8_t>(0x80u >> (x % 8));
    return (lineData[byteIndex] & bitMask) != 0;
  }

  static void WriteColor(const std::span<uint8_t> &lineData, BitmapSizeType x, bool color) {
    const auto byteIndex = x / 8u;
    if (byteIndex >= lineData.size())
      return;

    const auto bitMask = static_cast<uint8_t>(0x80u >> (x % 8));
    if (color) {
      lineData[byteIndex] |= bitMask;
    } else {
      lineData[byteIndex] &= static_cast<uint8_t>(~bitMask);
    }
  }
};

struct BitmapDepth8 {
  static constexpr uint8_t BitsPerPixel = 8;
  using Type = uint8_t;

  static constexpr DrawableSurface::RGBInfo DefaultShift = {0, 0, 0};
  static constexpr DrawableSurface::RGBInfo DefaultMask = {0, 0, 0};

  static constexpr size_t BufferBytesPerLine(size_t width) { return ((width * BitsPerPixel + 31u) / 32u) * 4u; }
  static constexpr size_t ValidBytesPerLine(size_t width) { return width; }

  static uint8_t ReadColor(std::span<const uint8_t> lineData, BitmapSizeType x) {
    return lineData[x];
  }
  static void WriteColor(const std::span<uint8_t> &lineData, BitmapSizeType x, uint8_t color) {
    lineData[x] = color;
  }
};

struct BitmapDepth16 {
  static constexpr uint8_t BitsPerPixel = 16;
  using Type = uint16_t;

  static constexpr DrawableSurface::RGBInfo DefaultShift = {11, 5, 0};
  static constexpr DrawableSurface::RGBInfo DefaultMask = {0x1F, 0x3F, 0x1F};// 5:6:5

  static constexpr size_t BufferBytesPerLine(size_t width) { return ((width * BitsPerPixel + 31u) / 32u) * 4u; }
  static constexpr size_t ValidBytesPerLine(size_t width) { return width * sizeof(Type); }

  static Color ReadColor(std::span<const uint8_t> lineData,
                         BitmapSizeType x,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return {};

    const Type raw = load_pixel<Type>(lineData, x);
    return {
        detail::ExtractAndScaleChannel(raw, shift.red, mask.red),
        detail::ExtractAndScaleChannel(raw, shift.green, mask.green),
        detail::ExtractAndScaleChannel(raw, shift.blue, mask.blue)};
  }

  static void WriteColor(const std::span<uint8_t> &lineData,
                         BitmapSizeType x,
                         const Color &color,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return;

    Type dstColor = 0;
    dstColor |= detail::CompressAndShiftChannel<Type>(color.red, shift.red, mask.red);
    dstColor |= detail::CompressAndShiftChannel<Type>(color.green, shift.green, mask.green);
    dstColor |= detail::CompressAndShiftChannel<Type>(color.blue, shift.blue, mask.blue);

    store_pixel<Type>(lineData, x, dstColor);
  }
};

struct BitmapDepth32 {
  static constexpr uint8_t BitsPerPixel = 32;
  using Type = uint32_t;

  static constexpr DrawableSurface::RGBInfo DefaultShift = {16, 8, 0};
  static constexpr DrawableSurface::RGBInfo DefaultMask = {0xFF, 0xFF, 0xFF};// XRGB

  static constexpr size_t BufferBytesPerLine(size_t width) { return ((width * BitsPerPixel + 31u) / 32u) * 4u; }
  static constexpr size_t ValidBytesPerLine(size_t width) { return width * sizeof(Type); }

  static Color ReadColor(std::span<const uint8_t> lineData,
                         BitmapSizeType x,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return {};

    const Type raw = load_pixel<Type>(lineData, x);
    return {
        detail::ExtractAndScaleChannel(raw, shift.red, mask.red),
        detail::ExtractAndScaleChannel(raw, shift.green, mask.green),
        detail::ExtractAndScaleChannel(raw, shift.blue, mask.blue)};
  }

  static void WriteColor(const std::span<uint8_t> &lineData,
                         BitmapSizeType x,
                         const Color &color,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return;

    Type dstColor = 0;
    dstColor |= detail::CompressAndShiftChannel<Type>(color.red, shift.red, mask.red);
    dstColor |= detail::CompressAndShiftChannel<Type>(color.green, shift.green, mask.green);
    dstColor |= detail::CompressAndShiftChannel<Type>(color.blue, shift.blue, mask.blue);
    dstColor |= detail::CompressAndShiftChannel<Type>(0xFF, 24, 0xFF);

    store_pixel<Type>(lineData, x, dstColor);
  }
};

template<DrawableSurface::BitDepth>
struct BitmapHelper;

template<>
struct BitmapHelper<DrawableSurface::BitDepth::DEPTH_1> {
  using type = BitmapDepth1;
};

template<>
struct BitmapHelper<DrawableSurface::BitDepth::DEPTH_8> {
  using type = BitmapDepth8;
};

template<>
struct BitmapHelper<DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE> {
  using type = BitmapDepth8;
};

template<>
struct BitmapHelper<DrawableSurface::BitDepth::DEPTH_16> {
  using type = BitmapDepth16;
};

template<>
struct BitmapHelper<DrawableSurface::BitDepth::DEPTH_32> {
  using type = BitmapDepth32;
};

// Convenience alias
template<DrawableSurface::BitDepth D>
using BitmapHelper_t = typename BitmapHelper<D>::type;

constexpr size_t GetBitmapBufferBytesPerLine(DrawableSurface::BitDepth depth, size_t width) {
  switch (depth) {
    case DrawableSurface::BitDepth::DEPTH_1: return BitmapDepth1::BufferBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_8:
    case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: return BitmapDepth8::BufferBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_16: return BitmapDepth16::BufferBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_32: return BitmapDepth32::BufferBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_INVALID: return 0;
  }
  return 0;
}

constexpr size_t GetBitmapValidBytesPerLine(DrawableSurface::BitDepth depth, size_t width) {
  switch (depth) {
    case DrawableSurface::BitDepth::DEPTH_1: return BitmapDepth1::ValidBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_8:
    case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: return BitmapDepth8::ValidBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_16: return BitmapDepth16::ValidBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_32: return BitmapDepth32::ValidBytesPerLine(width);
    case DrawableSurface::BitDepth::DEPTH_INVALID: return 0;
  }
  return 0;
}

constexpr size_t GetBufferSizeForBitmapSize(DrawableSurface::BitDepth depth, size_t width, size_t height) {
  return GetBitmapBufferBytesPerLine(depth, width) * height;
}

}// namespace e00::helpers
