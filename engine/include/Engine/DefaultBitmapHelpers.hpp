
#pragma once

#include <bit>
#include <cstdint>
#include <cstring>
#include <span>

#include <Engine/Math/Color.hpp>
#include <Engine/Platform/DrawableSurface.hpp>

namespace e00::helpers {
template<typename T>
constexpr uint8_t ExtractAndScaleChannel(T rawcolor, uint32_t shift, uint32_t mask) {
  if (mask == 0) [[unlikely]]
    return 0;

  const uint32_t val = (static_cast<uint32_t>(rawcolor) >> shift) & mask;

  // Fast path for common contiguous masks
  switch (mask) {
    case 0xFFu: return static_cast<uint8_t>(val);                    // 8-bit
    case 0x1Fu: return static_cast<uint8_t>((val << 3) | (val >> 2));// 5-bit -> 8-bit
    case 0x3Fu: return static_cast<uint8_t>((val << 2) | (val >> 4));// 6-bit -> 8-bit
    default:
      [[unlikely]] {
        // Contiguous mask guaranteed
        const int bits = std::bit_width(mask);// 1..32

        if (bits >= 8) {
          const int s = bits - 8;// 0..24
          return static_cast<uint8_t>(val >> s);
        }

        // bits < 8: cheap replication; white/black preserved, midtones "good enough"
        const int shift_up = 8 - bits;// 1..7
        uint32_t v = val << shift_up;
        v |= v >> bits;// smear high bits into low
        return static_cast<uint8_t>(v);
      }
  }
}

template<typename T>
constexpr T CompressAndShiftChannel(uint8_t color_component, uint32_t shift, uint32_t mask) {
  if (mask == 0) [[unlikely]]
    return 0;

  uint32_t val = color_component;

  switch (mask) {
    case 0xFFu: break;           // 8-bit
    case 0x1Fu: val >>= 3; break;// 8 -> 5
    case 0x3Fu: val >>= 2; break;// 8 -> 6
    default:
      [[unlikely]] {
        const int bits = std::bit_width(mask);// 1..32

        if (bits < 8) {
          const int s = 8 - bits;// 1..7
          val >>= s;
        } else if (bits > 8) {
          const int s = bits - 8;// 1..24
          val <<= s;
        }
        break;
      }
  }

  val &= mask;
  return static_cast<T>(val << shift);
}

template<typename T>
T load_pixel(std::span<const uint8_t> lineData, std::size_t x) {
  T v{};
  std::memcpy(&v, lineData.data() + x * sizeof(T), sizeof(T));
  return v;
}

template<typename T>
void store_pixel(std::span<uint8_t> &lineData, std::size_t x, T v) {
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
      return false;// or your preferred "out of range" behavior

    const auto bitMask = static_cast<uint8_t>(0x80u >> (x % 8));
    return (lineData[byteIndex] & bitMask) != 0;
  }

  static void WriteColor(std::span<uint8_t> &lineData, BitmapSizeType x, bool color) {
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
  static void WriteColor(std::span<uint8_t> &lineData, BitmapSizeType x, uint8_t color) {
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
        ExtractAndScaleChannel(raw, shift.red, mask.red),
        ExtractAndScaleChannel(raw, shift.green, mask.green),
        ExtractAndScaleChannel(raw, shift.blue, mask.blue)};
  }

  static void WriteColor(std::span<uint8_t> &lineData,
                         BitmapSizeType x,
                         const Color &color,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return;

    Type dstColor = 0;
    dstColor |= CompressAndShiftChannel<Type>(color.red, shift.red, mask.red);
    dstColor |= CompressAndShiftChannel<Type>(color.green, shift.green, mask.green);
    dstColor |= CompressAndShiftChannel<Type>(color.blue, shift.blue, mask.blue);

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
        ExtractAndScaleChannel(raw, shift.red, mask.red),
        ExtractAndScaleChannel(raw, shift.green, mask.green),
        ExtractAndScaleChannel(raw, shift.blue, mask.blue)};
  }

  static void WriteColor(std::span<uint8_t> &lineData,
                         BitmapSizeType x,
                         const Color &color,
                         const DrawableSurface::RGBInfo &shift = DefaultShift,
                         const DrawableSurface::RGBInfo &mask = DefaultMask) {
    if (x >= lineData.size() / sizeof(Type))
      return;

    Type dstColor = 0;
    dstColor |= CompressAndShiftChannel<Type>(color.red, shift.red, mask.red);
    dstColor |= CompressAndShiftChannel<Type>(color.green, shift.green, mask.green);
    dstColor |= CompressAndShiftChannel<Type>(color.blue, shift.blue, mask.blue);

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

}// namespace e00::helpers
