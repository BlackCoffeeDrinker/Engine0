#pragma once

#include "PrivateInclude.hpp"

namespace e00 {
class SoftwarePainter : public Painter {
  Bitmap &_target;

  void PutPixel(BitmapSizeType x, BitmapSizeType y, const Color &color) const;
  void PutPixel(BitmapSizeType x, BitmapSizeType y, uint8_t index) const;
  void Copy8BitNoPalette(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) const;
  void Copy8BitTo8Bit(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) const;
  void DrawGenericData(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) const;

public:
  explicit SoftwarePainter(Bitmap &target) : _target(target) {}

  [[nodiscard]] DrawableSurface::TargetInformation GetTargetInformation() const override {
    return {
        .bit_depth = _target.GetBitDepth(),
        .palette = &_target._palette,
        .shift = _target.GetShift(),
        .mask = _target.GetMask(),
    };
  }

  [[nodiscard]] BitmapSize GetDrawableSize() const override { return _target.Size(); }

  void DrawPoint(const Vec2D<BitmapSizeType> &pos) override;
  void DrawEllipse(const RectT<BitmapSizeType> &rect) override;
  void DrawRect(const RectT<BitmapSizeType> &rect) override;
  void DrawLine(const Vec2D<BitmapSizeType> &start, const Vec2D<BitmapSizeType> &end) override;
  void BlitRawLine(BitmapSizeType line, BitmapSizeType startX, BitmapSizeType endX, const std::span<const uint8_t> &data, const DrawableSurface::TargetInformation &dataFormatting) override;
  void BlitMaskedLine(BitmapSizeType line, BitmapSizeType startX, BitmapSizeType endX, const std::span<const uint8_t> &data, const std::span<const uint8_t> &mask, const DrawableSurface::TargetInformation &dataFormatting) override;
  void BlitSurface(const DrawableSurface &src,
                   RectT<BitmapSizeType> srcRect,
                   Vec2D<BitmapSizeType> dstPos) override;
};

}// namespace e00
