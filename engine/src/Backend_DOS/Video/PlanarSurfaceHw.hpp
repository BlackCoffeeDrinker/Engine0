
#pragma once

#include "Platform.hpp"

#include <span>
#include <vector>

namespace DOS {

// --- 1. THE SHARED PLANAR BASE CLASS ---
class PlanarSurfaceHw : public platform::Surface {
protected:
  uint16_t _width;
  uint16_t _height;
  size_t _bytes_per_line;
  uint8_t *_vram_address;            // Direct pointer into the A0000 video aperture
  int _pixels_per_byte_shift;        // 3 for Mode 12h (div 8), 2 for Mode X/Y (div 4)
  std::vector<uint8_t> _ram_fallback;// Used only if VRAM allocation fails
  e00::FixedPalette _palette;        // Local copy of the active hardware palette

public:
  PlanarSurfaceHw(uint16_t w, uint16_t h, size_t bpl, uint8_t *addr, int shift)
      : _width(w), _height(h), _bytes_per_line(bpl),
        _vram_address(addr), _pixels_per_byte_shift(shift), _palette(shift == 3 ? 16 : 256) {
    // If no VRAM address is provided, automatically allocate system RAM instead
    if (_vram_address == nullptr) {
      _ram_fallback.resize(_bytes_per_line * _height * 4);
    }
  }

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<PlanarSurfaceHw>(); }
  [[nodiscard]] bool IsHardwareAccelerated() const override { return _vram_address != nullptr; }
  [[nodiscard]] e00::Vec2D<e00::BitmapSizeType> Size() const override { return {_width, _height}; }
  [[nodiscard]] BitDepth GetBitDepth() const override { return BitDepth::DEPTH_8; }
  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] e00::Color GetColorFromPalette(size_t index) const override { return _palette[index]; }
  [[nodiscard]] uint8_t GetClosestColor(const e00::Color &color) const override { return _palette.findClosestColorIndex(color); }
  
  void DiscardPalette() override { }
  void SetPalette(const e00::FixedPalette &palette) override;

  void ReadLineInto(
      e00::BitmapSizeType line,
      e00::BitmapSizeType startX, e00::BitmapSizeType endX,
      const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const override;

  [[nodiscard]] std::unique_ptr<e00::Painter> BeginDraw() override;

  uint8_t *GetHardwareAddress() const { return _vram_address; }
  size_t GetBytesPerLine() const { return _bytes_per_line; }
  int GetShift() const { return _pixels_per_byte_shift; }

  uint8_t *GetPlaneAddress(int plane) {
    if (IsHardwareAccelerated()) return _vram_address;
    return _ram_fallback.data() + (plane * _bytes_per_line * _height);
  }

  const uint8_t *GetPlaneAddress(int plane) const {
    if (IsHardwareAccelerated()) return _vram_address;
    return _ram_fallback.data() + (plane * _bytes_per_line * _height);
  }

  [[nodiscard]] std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) override;
};

}// namespace DOS
