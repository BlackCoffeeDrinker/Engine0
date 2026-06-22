#pragma once

#include "PrivateInclude.hpp"

namespace e00::impl {

/**
 * Class BitmapData
 */
class BitmapData {
  size_t bytes_per_line;
  size_t valid_data_per_line;
  std::vector<uint8_t> data;

  // Only valid if bit_depth is 16 or 32
  DrawableSurface::RGBInfo shift;
  DrawableSurface::RGBInfo mask;

public:
  enum class MemoryAlignment {
    NoAlignment,
    WordAlignment,
  };

  /**
   * Constructs an invalid bitmap data
   */
  BitmapData()
      : bytes_per_line(0),
        valid_data_per_line(0) {
  }

  /**
   * Copies (data included) another BitmapData
   * 
   * @param other The data to copy
   */
  BitmapData(const BitmapData &other)
      : bytes_per_line(other.bytes_per_line),
        valid_data_per_line(other.valid_data_per_line) {
    data.resize(other.data.size());

    memcpy(data.data(), other.data.data(), data.size());
  }

  /**
   * Creates a data area big enough to store a bitmap of the specified size
   * 
   * @param size the bitmap size
   * @param bit_depth the bitmap depth
   * @param lineAlignment the line alignment; none means data is packed tightly with no padding between the rows
   */
  BitmapData(const Vec2D<uint16_t> &size, DrawableSurface::BitDepth bit_depth, [[maybe_unused]] MemoryAlignment lineAlignment = MemoryAlignment::WordAlignment);

  ~BitmapData() = default;

  [[nodiscard]] const auto &GetShift() const noexcept { return shift; }
  [[nodiscard]] const auto &GetMask() const noexcept { return mask; }

  [[nodiscard]] bool ReadColor1(BitmapSizeType x, BitmapSizeType y) const;
  [[nodiscard]] uint8_t ReadColor8(BitmapSizeType x, BitmapSizeType y) const;
  [[nodiscard]] Color ReadColor16(BitmapSizeType x, BitmapSizeType y) const;
  [[nodiscard]] Color ReadColor32(BitmapSizeType x, BitmapSizeType y) const;

  void WriteColor1(BitmapSizeType x, BitmapSizeType y, bool color);
  void WriteColor8(BitmapSizeType x, BitmapSizeType y, uint8_t color);
  void WriteColor16(BitmapSizeType x, BitmapSizeType y, const Color &color);
  void WriteColor32(BitmapSizeType x, BitmapSizeType y, const Color &color);

  void ReadLineInto(
      BitmapSizeType line,
      BitmapSizeType startX, BitmapSizeType endX,
      const DrawableSurface::TargetInformation &targetInformation,
      DrawableSurface::BitDepth srcDepth, const FixedPalette &sourcePalette,
      std::span<uint8_t> targetBuffer) const;

  auto GetLineSpan(const uint16_t y) {
    return std::span(
        data.data() + y * bytes_per_line,
        (data.data() + y * bytes_per_line) + valid_data_per_line);
  }

  [[nodiscard]] auto GetLineSpan(const uint16_t y) const {
    return std::span(
        data.data() + y * bytes_per_line,
        (data.data() + y * bytes_per_line) + valid_data_per_line);
  }

  uint8_t *GetLine(uint16_t y) { return data.data() + y * bytes_per_line; }
  [[nodiscard]] const uint8_t *GetLine(uint16_t y) const { return data.data() + y * bytes_per_line; }
};

}// namespace e00::impl
