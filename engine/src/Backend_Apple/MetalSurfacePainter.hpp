
#pragma once
#include <Engine.hpp>

namespace apple {
/**
 * Class MetalSurfacePainter
 */
class MetalSurfacePainter : public e00::Painter {
  e00::FixedPalette &_palette;
  std::vector<uint8_t> &_target;
  size_t _bytes_per_line;
  size_t _valid_data_per_line;

  [[nodiscard]] e00::Color GetPenColor() const {
    switch (_penStyle) {
      case PenStyle::NoPen: return {};
      case PenStyle::SolidLineColor: return _penColor;
      case PenStyle::SolidLineIndex: return _palette[_penIndex];
    }
  }

  [[nodiscard]] e00::Color GetBrushColor() const {
    switch (_brushStyle) {
      case BrushStyle::NoBrush: return {};
      case BrushStyle::SolidBrushColor: return _brushColor;
      case BrushStyle::SolidBrushIndex: return _palette[_brushIndex];
    }
  }

  [[nodiscard]] std::span<const uint8_t> GetLineData(e00::BitmapSizeType y) const {
    const size_t offset = static_cast<size_t>(y) * _bytes_per_line;
    if (offset + _valid_data_per_line > _target.size()) {
      return {};
    }
    return std::span(
        _target.data() + offset,
        _valid_data_per_line);
  }

  [[nodiscard]] std::span<uint8_t> GetLineData(e00::BitmapSizeType y) {
    const size_t offset = static_cast<size_t>(y) * _bytes_per_line;
    if (offset + _valid_data_per_line > _target.size()) {
      return {};
    }
    return std::span(
        _target.data() + offset,
        _valid_data_per_line);
  }

public:
  MetalSurfacePainter(e00::FixedPalette &palette,
                      std::vector<uint8_t> &target,
                      size_t bytes_per_line,
                      size_t valid_data_per_line)
      : _palette(palette), _target(target), _bytes_per_line(bytes_per_line), _valid_data_per_line(valid_data_per_line) {}

  ~MetalSurfacePainter() override {}

  [[nodiscard]] e00::DrawableSurface::TargetInformation GetTargetInformation() const override {
    return {
        .bit_depth = e00::DrawableSurface::BitDepth::DEPTH_8,
        .palette = &_palette,
    };
  }

  [[nodiscard]] e00::BitmapSize GetDrawableSize() const override {
    return {
        static_cast<e00::BitmapSizeType>(_bytes_per_line),
        static_cast<e00::BitmapSizeType>(_target.size() / _bytes_per_line)};
  }

  void DrawPoint(const e00::Vec2D<unsigned short> &pos) override;
  void DrawLine(const e00::Vec2D<unsigned short> &start, const e00::Vec2D<unsigned short> &end) override;
  void DrawEllipse(const e00::RectT<unsigned short> &rect) override;
  void DrawRect(const e00::RectT<unsigned short> &rect) override;
  void BlitRawLine(e00::BitmapSizeType line, e00::BitmapSizeType startX, e00::BitmapSizeType endX, const std::span<const uint8_t> &data, const e00::DrawableSurface::TargetInformation &dataFormatting) override;
  void BlitSurface(const e00::DrawableSurface &src, e00::RectT<unsigned short> srcRect, e00::Vec2D<unsigned short> dstPos) override;
};
}// namespace apple
