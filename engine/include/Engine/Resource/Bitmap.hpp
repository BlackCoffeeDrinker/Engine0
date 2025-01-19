#pragma once

namespace e00 {

class Bitmap : public Resource {
  struct BitmapData;

public:
  enum class BitDepth {
    DEPTH_1,// 1-bit monochrome
    DEPTH_8,// 8-bit indexed color
    DEPTH_32,// 32-bit "true color"
  };

private:
  const Vec2D<uint16_t> _size;
  const BitDepth _bit_depth;

  std::unique_ptr<BitmapData> _data;
  FixedPalette _palette;

  [[nodiscard]] bool CanDoFastCopyFrom(const Bitmap &src, const RectT<uint16_t> &srcRect, const Vec2D<uint16_t> &dstPos) const noexcept;

public:
  NOT_COPYABLE(Bitmap);

  Bitmap(const Vec2D<uint16_t> &size, BitDepth bit_depth, int numColorsInPalette = 0);

  ~Bitmap() override;

  /**
   * @brief Sets the color of a palette entry.
   *
   * This function modifies the color of a specific palette entry by its index.
   *
   * @param index the palette index to change
   * @param color the color to set this index to
   * @return an error code if the index is out of range or no error if the
   *         index was changed successfully
   */
  std::error_code SetPaletteColor(std::size_t index, const Color &color) {
    if (index < _palette.size()) {
      _palette[index] = color;
      return {};
    }

    return std::make_error_code(std::errc::invalid_argument);
  }

  /**
   * @brief Retrieves the color of a specific palette entry.
   *
   * This function fetches the color associated with a particular palette index.
   *
   * @param index the palette index for which the color should be retrieved
   * @return the color at the specified index or an invalid color if the index is out of range
   */
  [[nodiscard]] Color GetPaletteColor(std::size_t index) const {
    if (index < _palette.size()) {
      return _palette[index];
    }
    return {};
  }

  /**
   * @brief Returns the size of the bitmap
   *
   * This function returns the size of the bitmap, this cannot change during
   * this bitmap's lifetime
   *
   * @return size of the bitmap
   */
  [[nodiscard]] Vec2D<uint16_t> Size() const { return _size; }

  [[nodiscard]] BitDepth GetBitDepth() const { return _bit_depth; }

  [[nodiscard]] bool HasSamePalette(const Bitmap &other) const;

  [[nodiscard]] std::unique_ptr<Bitmap> ConvertToDepth(BitDepth bit_depth) const;

  [[nodiscard]] std::unique_ptr<Bitmap> ConvertToDepthWithPalette(BitDepth bit_depth, const FixedPalette &palette) const;

  [[nodiscard]] std::unique_ptr<Bitmap> Clone(RectT<uint16_t> copyRect) const;
  
  void BlitFrom(const Bitmap &src, RectT<uint16_t> srcRect, Vec2D<uint16_t> dstPos);
};

void BitBlit(Bitmap &dst, Vec2D<uint16_t> dstPos, const Bitmap &src, RectT<uint16_t> srcRect);

}// namespace e00
