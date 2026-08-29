#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

#include <vector>

namespace win31 {

// 8-bit paletted surface. Prefers WinG (WinGCreateBitmap + WinGBitBlt) when
// WING32.DLL/WING.DLL is available; falls back to plain GDI CreateDIBSection +
// StretchDIBits/BitBlt otherwise.
class Win31Surface final : public platform::Surface {
  e00::Vec2D<e00::BitmapSizeType> _size{};
  e00::FixedPalette _palette;
  e00::DrawableSurface::BitDepth _bitDepth = e00::DrawableSurface::BitDepth::DEPTH_8;

  BITMAPINFO *_bmi = nullptr;
  HBITMAP _dib = nullptr;
  HDC _memDc = nullptr;
  uint8_t *_bits = nullptr; // points into WinG/GDI bitmap; owned by GDI/WinG
  size_t _stride = 0;
  bool _usingWinG = false;

  // Software working copy used by SoftwarePainter via an owned Bitmap
  std::unique_ptr<e00::Bitmap> _bitmap;

  bool CreatePresentResources();
  void DestroyPresentResources();
  void SyncDibFromBitmap();

public:
  Win31Surface(e00::BitmapSizeType width, e00::BitmapSizeType height,
               e00::DrawableSurface::BitDepth depth = e00::DrawableSurface::BitDepth::DEPTH_8);
  ~Win31Surface() override;

  Win31Surface(const Win31Surface &) = delete;
  Win31Surface &operator=(const Win31Surface &) = delete;

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<Win31Surface>(); }
  [[nodiscard]] e00::Vec2D<e00::BitmapSizeType> Size() const override { return _size; }
  [[nodiscard]] BitDepth GetBitDepth() const override { return _bitDepth; }
  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] e00::Color GetColorFromPalette(size_t index) const override;
  [[nodiscard]] uint8_t GetClosestColor(const e00::Color &color) const override;

  void DiscardPalette() override;
  void SetPalette(const e00::FixedPalette &palette) override;

  [[nodiscard]] std::unique_ptr<e00::Painter> BeginDraw() override;
  [[nodiscard]] std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(
      const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) override;

  void ReadLineInto(e00::BitmapSizeType line,
                    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                    const TargetInformation &targetInformation,
                    const std::span<uint8_t> &targetBuffer) const override;

  void ReadTransparencyMaskLineInto(e00::BitmapSizeType line,
                                    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                                    const std::span<uint8_t> &targetBuffer) const override;

  // Present the current software bitmap into the WinG/GDI bitmap and blit to a window DC.
  void Present(HDC windowDc, int destX, int destY, int destW, int destH);

  [[nodiscard]] bool UsingWinG() const { return _usingWinG; }
  [[nodiscard]] HBITMAP DibHandle() const { return _dib; }
  [[nodiscard]] const BITMAPINFO *Bmi() const { return _bmi; }
  [[nodiscard]] const uint8_t *Bits() const { return _bits; }
  [[nodiscard]] e00::Bitmap *SoftwareBitmap() { return _bitmap.get(); }
};

} // namespace win31
