#pragma once

#include <Engine/Resource/DrawableResource.hpp>

namespace e00 {
/**
 * Bitmap class provides a hardware-independent image
 * representation that allows direct access to the pixel data
 *
 * This is an in-memory format
 */
class Bitmap : public DrawableResource {
  friend class Sprite;

public:
  static std::unique_ptr<Bitmap> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette);
  static std::unique_ptr<Bitmap> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette = 0);

  Bitmap(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth)
      : DrawableResource(size, bit_depth) {}
  ~Bitmap() override;

  [[nodiscard]] type_t Type() const override { return type_id<Bitmap>(); }

  virtual void SetPalette(const FixedPalette &colors) = 0;
  [[nodiscard]] virtual std::error_code SetPaletteColor(std::size_t index, const Color &color) = 0;
  [[nodiscard]] virtual std::span<uint8_t> GetLineData(BitmapSizeType y) = 0;
  virtual void WriteLine_N(BitmapSizeType y, const std::span<uint8_t> &input, size_t size) = 0;
};

}// namespace e00
