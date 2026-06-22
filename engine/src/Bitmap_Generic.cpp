#include "BitmapData.hpp"
#include "Painter_PaintDevice.hpp"
#include "PrivateInclude.hpp"

#include <Engine/DefaultBitmapHelpers.hpp>

#include <span>
#include <utility>

namespace {
class SoftwareBitmap : public e00::Bitmap {
  e00::impl::BitmapData _data;
  e00::FixedPalette _palette;

public:
  SoftwareBitmap(const e00::Vec2D<e00::BitmapSizeType> &size, e00::Bitmap::BitDepth bit_depth, e00::FixedPalette palette)
      : Bitmap(size, bit_depth), _data(size, bit_depth), _palette(std::move(palette)) {
  }

  SoftwareBitmap(const e00::Vec2D<e00::BitmapSizeType> &size, e00::Bitmap::BitDepth bit_depth, int numColorsInPalette)
      : Bitmap(size, bit_depth), _data(size, bit_depth) {
    // Allocate palette?
    if (numColorsInPalette > 0) {
      // Validate the number of colors
      if (numColorsInPalette > 256) {
        abort();
      }

      _palette.resize(numColorsInPalette);
    }
  }
  
  void SetPalette(const e00::FixedPalette &colors) override { _palette = colors; }
  [[nodiscard]] std::error_code SetPaletteColor(std::size_t index, const e00::Color &color) override {
    if (index < _palette.size()) {
      _palette[index] = color;
      return {};
    }

    return std::make_error_code(std::errc::invalid_argument);
  }

  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] e00::Color GetColorFromPalette(size_t index) const override { return _palette[index]; }
  [[nodiscard]] uint8_t GetClosestColor(const e00::Color &color) const override { return _palette.findClosestColorIndex(color); }
  [[nodiscard]] std::unique_ptr<e00::Painter> BeginDraw() override { return std::make_unique<e00::SoftwarePainter>(Size(), GetBitDepth(), _palette, _data); }

  void ReadLineInto(e00::BitmapSizeType line,
                    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                    const TargetInformation &targetInformation,
                    std::span<uint8_t> targetBuffer) const override {
    _data.ReadLineInto(line, startX, endX, targetInformation, GetBitDepth(), _palette, targetBuffer);
  }

  std::span<uint8_t> GetLineData(e00::BitmapSizeType y) override { return _data.GetLineSpan(y); }

  void WriteLine_N(e00::BitmapSizeType y, const std::span<uint8_t> &input, size_t size) override {
    auto dstLine = _data.GetLineSpan(y);
    if (size > dstLine.size()) {
      size = dstLine.size();
    }
    std::memcpy(dstLine.data(), input.data(), size);
  }
};

}// namespace

namespace e00 {
Bitmap::~Bitmap() = default;

std::unique_ptr<Bitmap> Bitmap::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette) {
  return std::make_unique<SoftwareBitmap>(size, bit_depth, palette);
}

std::unique_ptr<Bitmap> Bitmap::Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette) {
  return std::make_unique<SoftwareBitmap>(size, bit_depth, numColorsInPalette);
}

}// namespace e00
