
#include "PrivateInclude.hpp"

namespace e00 {
void Painter::DrawRect(const RectT<BitmapSizeType> &rect) {
  // Since we're gonna change the pen, save it here
  auto oldPenStyle = _penStyle;
  auto oldPenWidth = _penWidth;
  auto oldPenColor = _penColor;
  auto oldPenIndex = _penIndex;

  if (_brushStyle != BrushStyle::NoBrush) {
    if (_brushStyle == BrushStyle::SolidBrushColor) {
      SetPenSolid(1, _brushColor);
    } else if (_brushStyle == BrushStyle::SolidBrushIndex) {
      SetPenSolid(1, _brushIndex);
    }

    for (BitmapSizeType y = 0; y < rect.size.y; ++y) {
      for (BitmapSizeType x = 0; x < rect.size.x; ++x) {
        DrawPoint({static_cast<unsigned short>(rect.origin.x + x),
                   static_cast<unsigned short>(rect.origin.y + y)});
      }
    }
  }

  switch (oldPenStyle) {
    case PenStyle::NoPen: SetNoPen(); break;
    case PenStyle::SolidLineColor: SetPenSolid(oldPenWidth, oldPenColor); break;
    case PenStyle::SolidLineIndex: SetPenSolid(oldPenWidth, oldPenIndex); break;
  }

  if (_penStyle != PenStyle::NoPen) {
    const auto x1 = rect.origin.x;
    const auto y1 = rect.origin.y;
    const auto x2 = static_cast<BitmapSizeType>(rect.origin.x + rect.size.x - 1);
    const auto y2 = static_cast<BitmapSizeType>(rect.origin.y + rect.size.y - 1);

    DrawLine({x1, y1}, {x2, y1});
    DrawLine({x1, y2}, {x2, y2});
    DrawLine({x1, y1}, {x1, y2});
    DrawLine({x2, y1}, {x2, y2});
  }
}

void Painter::DrawLine(const Vec2D<BitmapSizeType> &start, const Vec2D<BitmapSizeType> &end) {
  if (_penStyle == PenStyle::NoPen) return;

  int x0 = start.x;
  int y0 = start.y;
  const int x1 = end.x;
  const int y1 = end.y;

  const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  for (;;) {
    DrawPoint({static_cast<unsigned short>(x0), static_cast<unsigned short>(y0)});
    
    if (x0 == x1 && y0 == y1) break;
    const int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void Painter::DrawEllipse(const RectT<BitmapSizeType> &rect) {
  const long rx = rect.size.x / 2;
  const long ry = rect.size.y / 2;
  const long xc = rect.origin.x + rx;
  const long yc = rect.origin.y + ry;
  long x = 0, y = ry;
  const long rx2 = rx * rx, ry2 = ry * ry;
  long p = e00::lrint(ry2 - rx2 * ry + 0.25 * rx2);
  long dx = 2 * ry2 * x, dy = 2 * rx2 * y;

  // Since we're gonna change the pen, save it here
  auto oldPenStyle = _penStyle;
  auto oldPenWidth = _penWidth;
  auto oldPenColor = _penColor;
  auto oldPenIndex = _penIndex;

  auto plot_symmetrical = [&](long px, long py) {
    if (_brushStyle != BrushStyle::NoBrush) {
      for (long ix = xc - px; ix <= xc + px; ++ix) {
        if (_brushStyle == BrushStyle::SolidBrushIndex) {
          SetPenSolid(1, _brushIndex);
        } else {
          SetPenSolid(1, _brushColor);
        }
        DrawPoints({Vec2D<BitmapSizeType>(ix, yc + py),
                    Vec2D<BitmapSizeType>(ix, yc - py)});
      }
    }

    switch (oldPenStyle) {
      case PenStyle::NoPen: SetNoPen(); break;
      case PenStyle::SolidLineColor: SetPenSolid(oldPenWidth, oldPenColor); break;
      case PenStyle::SolidLineIndex: SetPenSolid(oldPenWidth, oldPenIndex); break;
    }

    if (_penStyle != PenStyle::NoPen) {
      DrawPoints({
          Vec2D<BitmapSizeType>(xc + px, yc + py),
          Vec2D<BitmapSizeType>(xc - px, yc + py),
          Vec2D<BitmapSizeType>(xc + px, yc - py),
          Vec2D<BitmapSizeType>(xc - px, yc - py),
      });
    }
  };

  while (dx < dy) {
    plot_symmetrical(x, y);
    if (p < 0) {
      x++;
      dx += 2 * ry2;
      p += dx + ry2;
    } else {
      x++;
      y--;
      dx += 2 * ry2;
      dy -= 2 * rx2;
      p += dx - dy + ry2;
    }
  }
  p = e00::lrint(ry2 * (x + 0.5) * (x + 0.5) + rx2 * (y - 1) * (y - 1) - rx2 * ry2);
  while (y >= 0) {
    plot_symmetrical(x, y);
    if (p > 0) {
      y--;
      dy -= 2 * rx2;
      p += rx2 - dy;
    } else {
      x++;
      y--;
      dx += 2 * ry2;
      dy -= 2 * rx2;
      p += dx - dy + rx2;
    }
  }
}

void Painter::DrawSurface(const DrawableSurface &src,
                         RectT<BitmapSizeType> srcRect,
                         Vec2D<BitmapSizeType> dstPos) {
  // Generic Path using 32-bit intermediate
  DrawableSurface::TargetInformation info32;
  info32.bit_depth = DrawableSurface::BitDepth::DEPTH_32;
  info32.shift = {16, 8, 0};
  info32.mask = {0xFF, 0xFF, 0xFF};

  std::vector<uint8_t> line32(srcRect.size.x * 4);

  // Since we're gonna change the pen, save it here
  auto oldPenStyle = _penStyle;
  auto oldPenWidth = _penWidth;
  auto oldPenColor = _penColor;
  auto oldPenIndex = _penIndex;

  for (BitmapSizeType y = 0; y < srcRect.size.y; ++y) {
    src.ReadLineInto(srcRect.origin.y + y, srcRect.origin.x, srcRect.origin.x + srcRect.size.x, info32, line32);
    for (BitmapSizeType x = 0; x < srcRect.size.x; ++x) {
      Color c{line32[x * 4 + 2], line32[x * 4 + 1], line32[x * 4 + 0]};
      SetPenSolid(1, c);
      DrawPoint({static_cast<unsigned short>(dstPos.x + x), static_cast<unsigned short>(dstPos.y + y)});
    }
  }

  switch (oldPenStyle) {
    case PenStyle::NoPen: SetNoPen(); break;
    case PenStyle::SolidLineColor: SetPenSolid(oldPenWidth, oldPenColor); break;
    case PenStyle::SolidLineIndex: SetPenSolid(oldPenWidth, oldPenIndex); break;
  }
}
}// namespace e00
