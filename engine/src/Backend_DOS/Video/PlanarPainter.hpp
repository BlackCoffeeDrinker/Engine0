#pragma once

#include "Platform.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <pc.h>

namespace DOS {
class PlanarPainter : public e00::Painter {
  static constexpr uint16_t GRAPHICS_MODE_REGISTERS = 0x3CE;

  e00::DrawableSurface &_target;
  e00::FixedPalette &_palette;
  uint16_t _width;
  uint16_t _height;
  size_t _pitch;
  uint8_t *_vram_base;
  bool _is_hardware;
  int _shift;// 3 for Mode 12h/EGA (div 8)

  // Hardware write helper using Write Mode 2
  void write_pixel_planar(int x, int y, uint8_t index);

public:
  PlanarPainter(e00::DrawableSurface &target,
                uint16_t w,
                uint16_t h,
                size_t pitch,
                uint8_t *vram,
                int shift,
                e00::FixedPalette &palette)
      : Painter(),
        _target(target),
        _palette(palette),
        _width(w), _height(h),
        _pitch(pitch),
        _vram_base(vram),
        _is_hardware(target.IsHardwareAccelerated()),
        _shift(shift) {
    if (_is_hardware) {
      // Put the hardware into Write Mode 2
      outportb(GRAPHICS_MODE_REGISTERS, 0x05);
      outportb(0x3CF, 0x02);
      outportb(GRAPHICS_MODE_REGISTERS, 0x03);
      outportb(0x3CF, 0x00);
    }
  }

  ~PlanarPainter() override {
    if (_is_hardware) {
      // Safe RAII Reset back to standard Write Mode 0
      outportb(GRAPHICS_MODE_REGISTERS, 0x05);
      outportb(0x3CF, 0x00);
      outportb(GRAPHICS_MODE_REGISTERS, 0x08);
      outportb(0x3CF, 0xFF);
    }
  }

  void DrawPoint(const e00::Vec2D<e00::BitmapSizeType> &pos) override;
  void DrawLine(const e00::Vec2D<e00::BitmapSizeType> &start, const e00::Vec2D<e00::BitmapSizeType> &end) override;
  void DrawRect(const e00::RectT<e00::BitmapSizeType> &rect) override;
  void DrawEllipse(const e00::RectT<e00::BitmapSizeType> &rect) override;
  void DrawSurface(const e00::DrawableSurface &src, e00::RectT<e00::BitmapSizeType> srcRect, e00::Vec2D<e00::BitmapSizeType> dstPos) override;
};

}// namespace DOS
