
#include "MetalSurfacePainter.hpp"
#include <cstring>

namespace apple {
void MetalSurfacePainter::DrawPoint(const e00::Vec2D<e00::BitmapSizeType> &pos) {
  switch (_penStyle) {
    case PenStyle::NoPen: break;
    case PenStyle::SolidLineColor: e00::helpers::BitmapDepth32::WriteColor(GetLineData(pos.y), pos.x, _penColor); break;
    case PenStyle::SolidLineIndex: e00::helpers::BitmapDepth32::WriteColor(GetLineData(pos.y), pos.x, _palette[_penIndex]); break;
  }
}

void MetalSurfacePainter::DrawLine(const e00::Vec2D<unsigned short> &start, const e00::Vec2D<unsigned short> &end) {
  if (_penStyle == PenStyle::NoPen) return;
  const auto lineColor = GetPenColor();

  int x0 = start.x;
  int y0 = start.y;
  const int x1 = end.x;
  const int y1 = end.y;

  const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  for (;;) {
    e00::helpers::BitmapDepth32::WriteColor(GetLineData(y0), x0, lineColor);

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

void MetalSurfacePainter::DrawEllipse(const e00::RectT<unsigned short> &rect) {
  bool doLine = _penStyle != PenStyle::NoPen;
  bool doFill = _brushStyle != BrushStyle::NoBrush;
  const auto fillColor = GetBrushColor();

  const long rx = rect.size.x / 2;
  const long ry = rect.size.y / 2;
  const long xc = rect.origin.x + rx;
  const long yc = rect.origin.y + ry;
  long x = 0, y = ry;
  const long rx2 = rx * rx, ry2 = ry * ry;
  long p = e00::lrint(ry2 - rx2 * ry + 0.25 * rx2);
  long dx = 2 * ry2 * x, dy = 2 * rx2 * y;

  auto plot_symmetrical = [&](long px, long py) {
    if (doFill) {
      auto topLine = GetLineData(static_cast<e00::BitmapSizeType>(yc + py));
      auto bottomLine = GetLineData(static_cast<e00::BitmapSizeType>(yc - py));
      for (long ix = xc - px; ix <= xc + px; ++ix) {
        e00::helpers::BitmapDepth32::WriteColor(topLine, static_cast<e00::BitmapSizeType>(ix), fillColor);
        e00::helpers::BitmapDepth32::WriteColor(bottomLine, static_cast<e00::BitmapSizeType>(ix), fillColor);
      }
    }

    if (doLine) {
      DrawPoint({static_cast<unsigned short>(xc + px), static_cast<unsigned short>(yc + py)});
      DrawPoint({static_cast<unsigned short>(xc - px), static_cast<unsigned short>(yc + py)});
      DrawPoint({static_cast<unsigned short>(xc + px), static_cast<unsigned short>(yc - py)});
      DrawPoint({static_cast<unsigned short>(xc - px), static_cast<unsigned short>(yc - py)});
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

void MetalSurfacePainter::DrawRect(const e00::RectT<unsigned short> &rect) {
  if (_brushStyle != BrushStyle::NoBrush) {
    const auto fillColor = GetBrushColor();
    for (e00::BitmapSizeType y = rect.origin.y; y < rect.origin.y + rect.size.y; ++y) {
      auto line = GetLineData(y);
      for (e00::BitmapSizeType x = rect.origin.x; x < rect.origin.x + rect.size.x; ++x) {
        e00::helpers::BitmapDepth32::WriteColor(line, x, fillColor);
      }
    }
  }

  if (_penStyle != PenStyle::NoPen) {
    const auto x1 = rect.origin.x;
    const auto y1 = rect.origin.y;
    const auto x2 = static_cast<unsigned short>(rect.origin.x + rect.size.x - 1);
    const auto y2 = static_cast<unsigned short>(rect.origin.y + rect.size.y - 1);

    DrawLine({x1, y1}, {x2, y1});
    DrawLine({x1, y2}, {x2, y2});
    DrawLine({x1, y1}, {x1, y2});
    DrawLine({x2, y1}, {x2, y2});
  }
}

void MetalSurfacePainter::BlitRawLine(e00::BitmapSizeType line, e00::BitmapSizeType startX, e00::BitmapSizeType endX, const std::span<const uint8_t> &data, const e00::DrawableSurface::TargetInformation &dataFormatting) {
  if (endX <= startX) return;

  if (const auto dst = GetLineData(line);
      !dst.empty()) {
    const auto maxPixelCount = std::min(static_cast<size_t>(endX - startX), dst.size() / 4 - std::min(dst.size() / 4, static_cast<size_t>(startX)));

    switch (dataFormatting.bit_depth) {
      case e00::DrawableSurface::BitDepth::DEPTH_1:
        // 1bit uses dataFormatting->palette
        if (dataFormatting.palette != nullptr) {
          const auto pixelCount = std::min(maxPixelCount, data.size() * 8);
          for (size_t i = 0; i < pixelCount; ++i) {
            const bool bitSet = e00::helpers::BitmapDepth1::ReadColor(data, static_cast<e00::BitmapSizeType>(i));
            e00::helpers::BitmapDepth32::WriteColor(dst, startX + i, (*dataFormatting.palette)[bitSet ? 1 : 0]);
          }
        }
        break;

      case e00::DrawableSurface::BitDepth::DEPTH_8:
        // 8bit uses dataFormatting->palette
        if (dataFormatting.palette != nullptr) {
          const auto pixelCount = std::min(maxPixelCount, data.size());
          for (size_t i = 0; i < pixelCount; ++i) {
            e00::helpers::BitmapDepth32::WriteColor(dst, startX + i, (*dataFormatting.palette)[data[i]]);
          }
        }
        break;

      case e00::DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: {
        // 8bit but uses local palette
        const auto pixelCount = std::min(maxPixelCount, data.size());
        for (size_t i = 0; i < pixelCount; ++i) {
          e00::helpers::BitmapDepth32::WriteColor(dst, startX + i, _palette[data[i]]);
        }
        break;
      }

      case e00::DrawableSurface::BitDepth::DEPTH_16: {
        const auto pixelCount = std::min(maxPixelCount, data.size() / sizeof(uint16_t));
        for (size_t i = 0; i < pixelCount; ++i) {
          const auto color = e00::helpers::BitmapDepth16::ReadColor(data, static_cast<e00::BitmapSizeType>(i), dataFormatting.shift, dataFormatting.mask);
          e00::helpers::BitmapDepth32::WriteColor(dst, startX + i, color);
        }
        break;
      }

      case e00::DrawableSurface::BitDepth::DEPTH_32: {
        const auto dstOffset = static_cast<size_t>(startX) * 4;
        if (dstOffset < dst.size()) {
          std::memcpy(dst.data() + dstOffset, data.data(), std::min(maxPixelCount * 4, std::min(dst.size() - dstOffset, data.size())));
        }
        break;
      }

      case e00::DrawableSurface::BitDepth::DEPTH_INVALID: std::abort();
    }
  }
}

void MetalSurfacePainter::BlitSurface(const e00::DrawableSurface &src,
                                      e00::RectT<unsigned short> srcRect,
                                      e00::Vec2D<unsigned short> dstPos) {
  const e00::DrawableSurface::TargetInformation dstInfo{e00::DrawableSurface::BitDepth::DEPTH_8, &_palette};

  std::vector<uint8_t> dst(srcRect.size.x);

  for (e00::BitmapSizeType y = 0; y < srcRect.size.y; ++y) {
    src.ReadLineInto(srcRect.From().y + y,
                     srcRect.From().x, srcRect.To().x,
                     dstInfo,
                     dst);

    for (size_t i = 0; i < dst.size(); ++i) {
      const auto value = dst[i];
      if (value >= _palette.size())
        continue;
      
      e00::helpers::BitmapDepth32::WriteColor(
          GetLineData(dstPos.y + y), dstPos.x + i,
          _palette[value]);
    }
  }
}
}// namespace apple
