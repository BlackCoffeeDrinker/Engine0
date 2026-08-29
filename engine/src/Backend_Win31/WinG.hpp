#pragma once

#include "Win32Types.hpp"

namespace win31 {

// Dynamically loaded WinG (WING32.DLL / WING.DLL) entry points.
// Primary path for fast 8-bit blits on Win32s; GDI is the fallback when WinG is absent.

using PFN_WinGRecommendDIBFormat = BOOL(WINAPI *)(BITMAPINFO *pbi);
using PFN_WinGCreateDC = HDC(WINAPI *)();
using PFN_WinGCreateBitmap = HBITMAP(WINAPI *)(HDC hdc, BITMAPINFO *pbmi, void **ppvBits);
using PFN_WinGBitBlt = BOOL(WINAPI *)(HDC hdcDest, int xDest, int yDest, int width, int height,
                                      HDC hdcSrc, int xSrc, int ySrc);
using PFN_WinGStretchBlt = BOOL(WINAPI *)(HDC hdcDest, int xDest, int yDest, int widthDest, int heightDest,
                                          HDC hdcSrc, int xSrc, int ySrc, int widthSrc, int heightSrc);
using PFN_WinGCreateHalftonePalette = HPALETTE(WINAPI *)();

struct WinGApi {
  HMODULE module = nullptr;
  PFN_WinGRecommendDIBFormat RecommendDIBFormat = nullptr;
  PFN_WinGCreateDC CreateDC = nullptr;
  PFN_WinGCreateBitmap CreateBitmap = nullptr;
  PFN_WinGBitBlt BitBlt = nullptr;
  PFN_WinGStretchBlt StretchBlt = nullptr;
  PFN_WinGCreateHalftonePalette CreateHalftonePalette = nullptr;

  [[nodiscard]] bool Available() const {
    return module && CreateDC && CreateBitmap && BitBlt;
  }
};

// Load once; safe to call repeatedly. Returns true if WinG is usable.
bool EnsureWinGLoaded();
const WinGApi &GetWinG();
void UnloadWinG();

} // namespace win31
