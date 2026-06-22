#pragma once

#include "BitmapData.hpp"
#include "PrivateInclude.hpp"

namespace e00 {
class SoftwarePainter : public Painter {
  const Vec2D<BitmapSizeType> _targetSize;
  const DrawableSurface::BitDepth _bit_depth;
  const FixedPalette &_palette;
  impl::BitmapData &_target;

  [[nodiscard]] std::span<uint8_t> GetTargetLine(BitmapSizeType y) const {
    if (y < _targetSize.y) {
      return _target.GetLineSpan(y);
    }

    return {};
  }

  void PutPixel(BitmapSizeType x, BitmapSizeType y, const Color &color);
  void PutPixel(BitmapSizeType x, BitmapSizeType y, uint8_t index);
  void Copy8BitNoPalette(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos);
  void Copy8BitTo8Bit(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos);

  void DrawGenericData(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos);

public:
  SoftwarePainter(
      const Vec2D<BitmapSizeType> &target_size,
      DrawableSurface::BitDepth bit_depth,
      FixedPalette &palette,
      impl::BitmapData &target)
      : _targetSize(target_size),
        _bit_depth(bit_depth),
        _palette(palette),
        _target(target) {}

  void DrawPoint(const Vec2D<BitmapSizeType> &pos) override;
  void DrawEllipse(const RectT<BitmapSizeType> &rect) override;
  void DrawRect(const RectT<BitmapSizeType> &rect) override;
  void DrawSurface(const DrawableSurface &src,
                   RectT<BitmapSizeType> srcRect,
                   Vec2D<BitmapSizeType> dstPos) override;
};

}// namespace e00
