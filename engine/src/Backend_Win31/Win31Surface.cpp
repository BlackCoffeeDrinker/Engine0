#include "Win31Surface.hpp"

#include "WinG.hpp"

#include <cstring>

namespace win31 {
namespace {
constexpr e00::BitmapSizeType kDefaultW = 640;
constexpr e00::BitmapSizeType kDefaultH = 480;

size_t DibStride(e00::BitmapSizeType width) {
  return (static_cast<size_t>(width) + 3u) & ~static_cast<size_t>(3u);
}

BITMAPINFO *AllocBmi(e00::BitmapSizeType width, e00::BitmapSizeType height) {
  const size_t bytes = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
  auto *raw = static_cast<uint8_t *>(::operator new(bytes));
  std::memset(raw, 0, bytes);
  auto *bmi = reinterpret_cast<BITMAPINFO *>(raw);
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = width;
  bmi->bmiHeader.biHeight = -static_cast<LONG>(height); // top-down preferred for our software buffer
  bmi->bmiHeader.biPlanes = 1;
  bmi->bmiHeader.biBitCount = 8;
  bmi->bmiHeader.biCompression = BI_RGB;
  bmi->bmiHeader.biClrUsed = 256;
  bmi->bmiHeader.biClrImportant = 256;
  return bmi;
}

void FillIdentityPalette(BITMAPINFO *bmi) {
  for (int i = 0; i < 256; ++i) {
    bmi->bmiColors[i].rgbRed = static_cast<BYTE>(i);
    bmi->bmiColors[i].rgbGreen = static_cast<BYTE>(i);
    bmi->bmiColors[i].rgbBlue = static_cast<BYTE>(i);
    bmi->bmiColors[i].rgbReserved = 0;
  }
}
} // namespace

bool Win31Surface::CreatePresentResources() {
  DestroyPresentResources();

  _stride = DibStride(_size.x);
  _bmi = AllocBmi(_size.x, _size.y);
  if (!_bmi) {
    return false;
  }
  FillIdentityPalette(_bmi);

  const WinGApi &wing = GetWinG();
  if (wing.Available()) {
    // Ask WinG for the ideal DIB format for the display, then force our size/8bpp.
    if (wing.RecommendDIBFormat) {
      BITMAPINFO recommended{};
      recommended.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
      if (wing.RecommendDIBFormat(&recommended)) {
        // Keep recommended orientation/bitfields where useful, but we need 8bpp indexed.
        if (recommended.bmiHeader.biBitCount == 8) {
          _bmi->bmiHeader.biHeight = recommended.bmiHeader.biHeight >= 0
                                         ? static_cast<LONG>(_size.y)
                                         : -static_cast<LONG>(_size.y);
        }
      }
    }

    _memDc = wing.CreateDC();
    if (_memDc) {
      _dib = wing.CreateBitmap(_memDc, _bmi, reinterpret_cast<void **>(&_bits));
      if (_dib && _bits) {
        SelectObject(_memDc, _dib);
        _usingWinG = true;
        std::memset(_bits, 0, _stride * static_cast<size_t>(_size.y));
        return true;
      }
    }

    // WinG path failed mid-way — clean and fall through to GDI.
    if (_memDc) {
      DeleteDC(_memDc);
      _memDc = nullptr;
    }
    if (_dib) {
      DeleteObject(_dib);
      _dib = nullptr;
    }
    _bits = nullptr;
    _usingWinG = false;
    e00::GetDefaultLogger().Warning(
        e00::source_location::current(),
        "WinGCreateBitmap failed; falling back to GDI CreateDIBSection");
  }

  // GDI fallback
  HDC screen = GetDC(nullptr);
  _dib = CreateDIBSection(screen, _bmi, DIB_RGB_COLORS, reinterpret_cast<void **>(&_bits), nullptr, 0);
  if (screen) {
    ReleaseDC(nullptr, screen);
  }
  if (!_dib || !_bits) {
    DestroyPresentResources();
    return false;
  }

  _memDc = CreateCompatibleDC(nullptr);
  if (_memDc) {
    SelectObject(_memDc, _dib);
  }
  _usingWinG = false;
  std::memset(_bits, 0, _stride * static_cast<size_t>(_size.y));
  return true;
}

void Win31Surface::DestroyPresentResources() {
  if (_memDc) {
    DeleteDC(_memDc);
    _memDc = nullptr;
  }
  if (_dib) {
    DeleteObject(_dib);
    _dib = nullptr;
  }
  if (_bmi) {
    ::operator delete(_bmi);
    _bmi = nullptr;
  }
  _bits = nullptr;
  _stride = 0;
  _usingWinG = false;
}

Win31Surface::Win31Surface(e00::BitmapSizeType width, e00::BitmapSizeType height,
                           e00::DrawableSurface::BitDepth depth)
    : _size{width == 0 ? kDefaultW : width, height == 0 ? kDefaultH : height},
      _palette(256),
      _bitDepth(depth == BitDepth::DEPTH_INVALID ? BitDepth::DEPTH_8 : depth) {
  _bitmap = e00::Bitmap::Create(_size, _bitDepth, 256);
  CreatePresentResources();
}

Win31Surface::~Win31Surface() {
  DestroyPresentResources();
}

e00::Color Win31Surface::GetColorFromPalette(size_t index) const {
  if (index >= _palette.size()) {
    return {};
  }
  return _palette[index];
}

uint8_t Win31Surface::GetClosestColor(const e00::Color &color) const {
  return _palette.findClosestColorIndex(color);
}

void Win31Surface::DiscardPalette() {
  _bitDepth = BitDepth::DEPTH_8_NO_PALETTE;
}

void Win31Surface::SetPalette(const e00::FixedPalette &palette) {
  _palette = palette;
  if (_bitmap) {
    _bitmap->SetPalette(palette);
  }
  if (!_bmi) {
    return;
  }

  const size_t count = palette.size() < 256 ? palette.size() : 256;
  for (size_t i = 0; i < count; ++i) {
    const auto c = palette[i];
    _bmi->bmiColors[i].rgbRed = c.red;
    _bmi->bmiColors[i].rgbGreen = c.green;
    _bmi->bmiColors[i].rgbBlue = c.blue;
    _bmi->bmiColors[i].rgbReserved = 0;
  }
  for (size_t i = count; i < 256; ++i) {
    _bmi->bmiColors[i] = RGBQUAD{};
  }

  if (_memDc) {
    SetDIBColorTable(_memDc, 0, 256, _bmi->bmiColors);
  }
}

std::unique_ptr<e00::Painter> Win31Surface::BeginDraw() {
  if (!_bitmap) {
    return nullptr;
  }
  return _bitmap->BeginDraw();
}

std::unique_ptr<e00::DrawableSurface> Win31Surface::CreateOptimizedSurface(
    const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement /*where*/) {
  return std::make_unique<Win31Surface>(size.x, size.y, _bitDepth);
}

void Win31Surface::ReadLineInto(e00::BitmapSizeType line,
                                e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                                const TargetInformation &targetInformation,
                                const std::span<uint8_t> &targetBuffer) const {
  if (_bitmap) {
    _bitmap->ReadLineInto(line, startX, endX, targetInformation, targetBuffer);
  }
}

void Win31Surface::ReadTransparencyMaskLineInto(e00::BitmapSizeType line,
                                                e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                                                const std::span<uint8_t> &targetBuffer) const {
  if (_bitmap) {
    _bitmap->ReadTransparencyMaskLineInto(line, startX, endX, targetBuffer);
  }
}

void Win31Surface::SyncDibFromBitmap() {
  if (!_bitmap || !_bits) {
    return;
  }

  std::vector<uint8_t> line(static_cast<size_t>(_size.x));
  e00::DrawableSurface::TargetInformation info{
      .bit_depth = BitDepth::DEPTH_8,
      .palette = &_palette,
  };

  // WinG/GDI may use bottom-up DIBs (positive biHeight). Detect and flip.
  const bool bottomUp = _bmi && _bmi->bmiHeader.biHeight > 0;

  for (e00::BitmapSizeType y = 0; y < _size.y; ++y) {
    _bitmap->ReadLineInto(y, 0, _size.x, info, std::span<uint8_t>(line.data(), line.size()));
    const size_t dstY = bottomUp ? static_cast<size_t>(_size.y - 1 - y) : static_cast<size_t>(y);
    uint8_t *dst = _bits + dstY * _stride;
    std::memcpy(dst, line.data(), line.size());
    if (_stride > line.size()) {
      std::memset(dst + line.size(), 0, _stride - line.size());
    }
  }
}

void Win31Surface::Present(HDC windowDc, int destX, int destY, int destW, int destH) {
  if (!windowDc || !_bits || !_bmi) {
    return;
  }

  SyncDibFromBitmap();

  const int srcW = static_cast<int>(_size.x);
  const int srcH = static_cast<int>(_size.y);
  if (destW <= 0) destW = srcW;
  if (destH <= 0) destH = srcH;

  const WinGApi &wing = GetWinG();
  if (_usingWinG && _memDc && wing.Available()) {
    if (destW == srcW && destH == srcH && wing.BitBlt) {
      wing.BitBlt(windowDc, destX, destY, destW, destH, _memDc, 0, 0);
      return;
    }
    if (wing.StretchBlt) {
      wing.StretchBlt(windowDc, destX, destY, destW, destH, _memDc, 0, 0, srcW, srcH);
      return;
    }
  }

  // GDI fallback present path
  if (_memDc && destW == srcW && destH == srcH) {
    BitBlt(windowDc, destX, destY, destW, destH, _memDc, 0, 0, SRCCOPY);
  } else {
    StretchDIBits(windowDc,
                  destX, destY, destW, destH,
                  0, 0, srcW, srcH,
                  _bits, _bmi, DIB_RGB_COLORS, SRCCOPY);
  }
}

} // namespace win31
