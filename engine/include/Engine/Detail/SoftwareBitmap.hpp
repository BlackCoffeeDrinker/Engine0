
#pragma once

#include <Engine/Platform/DrawableSurface.hpp>
#include <bit>

namespace e00::detail {
constexpr uint8_t ExtractAndScaleChannel(uint32_t rawcolor, uint32_t shift, uint32_t mask) {
  if (mask == 0) [[unlikely]]
    return 0;

  const uint32_t val = (rawcolor >> shift) & mask;

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

struct SoftwareBitmapHelper {
  DrawableSurface::BitDepth bit_depth;

  // Only valid if bit_depth is 16 or 32
  DrawableSurface::RGBInfo shift{};
  DrawableSurface::RGBInfo mask{};
  size_t bytes_per_line{0};
  size_t valid_data_per_line{0};
  size_t buffer_size{0};

  SoftwareBitmapHelper();
  SoftwareBitmapHelper(DrawableSurface::BitDepth bit_depth, BitmapSizeType width, BitmapSizeType height);
  SoftwareBitmapHelper(DrawableSurface::BitDepth bit_depth, const BitmapSize &size) : SoftwareBitmapHelper(bit_depth, size.x, size.y) {}

  explicit operator bool() const { return bit_depth != DrawableSurface::BitDepth::DEPTH_INVALID && bytes_per_line != 0; }

  [[nodiscard]] DrawableSurface::TargetInformation GetTargetInformation() const {
    return {bit_depth, nullptr, shift, mask};
  }

  [[nodiscard]] size_t BufferPosition(const BitmapPosition &position) const {
    switch (bit_depth) {
      // DEPTH_1 packs 8 pixels per uint8_t
      case DrawableSurface::BitDepth::DEPTH_1: return position.y * bytes_per_line + position.x / 8;
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: return position.y * bytes_per_line + position.x;
      case DrawableSurface::BitDepth::DEPTH_16: return position.y * bytes_per_line + position.x * sizeof(uint16_t);
      case DrawableSurface::BitDepth::DEPTH_32: return position.y * bytes_per_line + position.x * sizeof(uint32_t);
      case DrawableSurface::BitDepth::DEPTH_INVALID: break;
    }
    return {};
  }

  [[nodiscard]] uint32_t ReadRaw(const std::span<const uint8_t> &data, const BitmapPosition &position) const {
    const auto byteIndex = BufferPosition(position);
    if (byteIndex >= data.size())
      return 0;

    switch (bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_1: {
        const auto bitMask = static_cast<uint8_t>(0x80u >> (position.x % 8));
        return (data[byteIndex] & bitMask) != 0;
      }
      case DrawableSurface::BitDepth::DEPTH_8: break;
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: return data[byteIndex];

      case DrawableSurface::BitDepth::DEPTH_16: {
        uint16_t raw = 0;
        std::memcpy(&raw, data.data() + byteIndex, sizeof(uint16_t));
        return raw;
      }
      case DrawableSurface::BitDepth::DEPTH_32: {
        uint32_t raw = 0;
        std::memcpy(&raw, data.data() + byteIndex, sizeof(uint32_t));
        return raw;
      }
      case DrawableSurface::BitDepth::DEPTH_INVALID: return 0;
    }
    return {};
  }

  [[nodiscard]] Color ReadColor(const std::span<const uint8_t> &data, FixedPalette *palette, const BitmapPosition &position) const {
    const auto raw = ReadRaw(data, position);
    switch (bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_1:
        assert(palette);
        return raw ? (*palette)[1] : (*palette)[0];
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
        assert(palette);
        return (*palette)[raw];
      case DrawableSurface::BitDepth::DEPTH_16:
      case DrawableSurface::BitDepth::DEPTH_32: return {
          ExtractAndScaleChannel(raw, shift.red, mask.red),
          ExtractAndScaleChannel(raw, shift.green, mask.green),
          ExtractAndScaleChannel(raw, shift.blue, mask.blue)};
      case DrawableSurface::BitDepth::DEPTH_INVALID: break;
    }
    return {};
  }

  void WriteRaw(const std::span<uint8_t> &data, const BitmapPosition &position, uint32_t value) const {
    const auto byteIndex = BufferPosition(position);
    if (byteIndex >= data.size())
      return;

    switch (bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_1: {
        const auto bitMask = static_cast<uint8_t>(0x80u >> (position.y % 8));
        if (value > 0) {
          data[byteIndex] |= bitMask;
        } else {
          data[byteIndex] &= static_cast<uint8_t>(~bitMask);
        }
        return;
      }
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: {
        assert(value <= 255);
        data[byteIndex] = static_cast<uint8_t>(value);
        return;
      }
      case DrawableSurface::BitDepth::DEPTH_16: {
        assert(value <= 65535);
        const uint16_t value16 = static_cast<uint16_t>(value);
        std::memcpy(data.data() + byteIndex, &value16, sizeof(value16));
        return;
      }
      case DrawableSurface::BitDepth::DEPTH_32: {
        std::memcpy(data.data() + byteIndex, &value, sizeof(uint32_t));
        return;
      }
      case DrawableSurface::BitDepth::DEPTH_INVALID: break;
    }
  }

  void WriteColor(const std::span<uint8_t> &data, FixedPalette *palette, const BitmapPosition &position, Color color) const {
    uint32_t raw = 0;
    switch (bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_1:
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
        assert(palette);
        raw = palette->findClosestColorIndex(color);
        break;
      case DrawableSurface::BitDepth::DEPTH_16:
        raw = CompressAndShiftChannel<uint16_t>(color.red, shift.red, mask.red) |
              CompressAndShiftChannel<uint16_t>(color.green, shift.green, mask.green) |
              CompressAndShiftChannel<uint16_t>(color.blue, shift.blue, mask.blue);
        break;
      case DrawableSurface::BitDepth::DEPTH_32:
        raw = CompressAndShiftChannel<uint32_t>(color.red, shift.red, mask.red) |
              CompressAndShiftChannel<uint32_t>(color.green, shift.green, mask.green) |
              CompressAndShiftChannel<uint32_t>(color.blue, shift.blue, mask.blue) |
              CompressAndShiftChannel<uint32_t>(0xFF, 24, 0xFF);
        break;
      case DrawableSurface::BitDepth::DEPTH_INVALID: return;
    }

    WriteRaw(data, position, raw);
  }

  void WriteIndex(const std::span<uint8_t> &data, const BitmapPosition &position, uint8_t index) const {
    uint32_t raw = 0;
    switch (bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_1:
        raw = index ? 1 : 0;
        break;
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
        raw = index;
        break;
      case DrawableSurface::BitDepth::DEPTH_16:
      case DrawableSurface::BitDepth::DEPTH_32:
      case DrawableSurface::BitDepth::DEPTH_INVALID: std::abort();
    }

    WriteRaw(data, position, raw);
  }

  template<typename T>
  [[nodiscard]] std::span<T> GetLineData(const std::span<T> &data, BitmapSizeType y) const {
#ifndef NDEBUG
    assert(valid_data_per_line != 0);
    assert(bytes_per_line != 0);
#endif

    return data.subspan(y * bytes_per_line, valid_data_per_line);
  }

  template<typename T>
  [[nodiscard]] std::span<T> GetLineData(const std::span<T> &data, BitmapSizeType y) {
#ifndef NDEBUG
    assert(valid_data_per_line != 0);
    assert(bytes_per_line != 0);
#endif

    return data.subspan(y * bytes_per_line, valid_data_per_line);
  }
};

}// namespace e00::detail
