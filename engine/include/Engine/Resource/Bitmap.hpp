#pragma once

#include <Engine/Detail/SoftwareBitmap.hpp>
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
  friend class SoftwarePainter;

  detail::SoftwareBitmapHelper helper;
  FixedPalette _palette;
  std::vector<uint8_t> _data;

  [[nodiscard]] Color ReadColorAt(BitmapSizeType x, BitmapSizeType y) const;

public:
  enum class MemoryAlignment {
    NoAlignment,
    WordAlignment,
  };

  static std::unique_ptr<Bitmap> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette);
  static std::unique_ptr<Bitmap> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette = 0);

  Bitmap(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, MemoryAlignment lineAlignment = MemoryAlignment::WordAlignment);
  ~Bitmap() override;

  [[nodiscard]] type_t Type() const override { return type_id<Bitmap>(); }
  [[nodiscard]] size_t SizeUsage() override { return sizeof(*this) + _data.size() + _palette.SizeUsage(); }
  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] Color GetColorFromPalette(size_t index) const override { return _palette[index]; }
  [[nodiscard]] uint8_t GetClosestColor(const Color &color) const override { return _palette.findClosestColorIndex(color); }

  [[nodiscard]] const auto &GetShift() const noexcept { return helper.shift; }
  [[nodiscard]] const auto &GetMask() const noexcept { return helper.mask; }
  void SetPalette(const FixedPalette &colors) { _palette = colors; }

  void ReadLineInto(BitmapSizeType line, BitmapSizeType startX, BitmapSizeType endX, const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const override;

  [[nodiscard]] std::error_code SetPaletteColor(std::size_t index, const Color &color) {
    if (index < _palette.size()) [[likely]] {
      _palette[index] = color;
      return {};
    }

    return std::make_error_code(std::errc::invalid_argument);
  }

  [[nodiscard]] std::unique_ptr<Painter> BeginDraw() override;
};

}// namespace e00
