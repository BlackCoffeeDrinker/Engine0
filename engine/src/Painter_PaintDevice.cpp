#include "Painter_PaintDevice.hpp"
#include "PrivateInclude.hpp"

namespace e00 {

void SoftwarePainter::PutPixel(BitmapSizeType x, BitmapSizeType y, const Color &color) {
  if (x >= _targetSize.x || y >= _targetSize.y) return;
  if (std::span<uint8_t> line = GetTargetLine(y);
      !line.empty()) {
    switch (_bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_32: helpers::BitmapDepth32::WriteColor(line, x, color, _target.GetShift(), _target.GetMask()); break;
      case DrawableSurface::BitDepth::DEPTH_16: helpers::BitmapDepth16::WriteColor(line, x, color, _target.GetShift(), _target.GetMask()); break;
      case DrawableSurface::BitDepth::DEPTH_8: helpers::BitmapDepth8::WriteColor(line, x, _palette.findClosestColorIndex(color)); break;
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
        GetDefaultLogger().Error(source_location::current(), "Cannot draw RGB color to DEPTH_8_NO_PALETTE surface");
        std::abort();
      case DrawableSurface::BitDepth::DEPTH_1: helpers::BitmapDepth1::WriteColor(line, x, _palette.findClosestColorIndex(color)); break;
      default: break;
    }
  }
}

void SoftwarePainter::PutPixel(BitmapSizeType x, BitmapSizeType y, uint8_t index) {
  if (x >= _targetSize.x || y >= _targetSize.y) return;
  if (std::span<uint8_t> line = GetTargetLine(y);
      !line.empty()) {
    switch (_bit_depth) {
      case DrawableSurface::BitDepth::DEPTH_32: helpers::BitmapDepth32::WriteColor(line, x, _palette[index], _target.GetShift(), _target.GetMask()); break;
      case DrawableSurface::BitDepth::DEPTH_16: helpers::BitmapDepth16::WriteColor(line, x, _palette[index], _target.GetShift(), _target.GetMask()); break;
      case DrawableSurface::BitDepth::DEPTH_8:
      case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
        helpers::BitmapDepth8::WriteColor(line, x, index);
        break;
      case DrawableSurface::BitDepth::DEPTH_1: helpers::BitmapDepth1::WriteColor(line, x, index > 0); break;
      default: break;
    }
  }
}

void SoftwarePainter::Copy8BitNoPalette(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) {
  const BitmapSizeType width = srcRect.size.x;
  const BitmapSizeType height = srcRect.size.y;

  // Direct copy for NO_PALETTE cases
  const DrawableSurface::TargetInformation info8{_bit_depth, nullptr};// targetPalette doesn't matter for NO_PALETTE copy in ReadLineInto

  for (BitmapSizeType y = 0; y < height; ++y) {
    if (auto targetLine = _target.GetLineSpan(dstPos.y + y); !targetLine.empty()) {
      src.ReadLineInto(
          srcRect.origin.y + y,
          srcRect.origin.x,
          srcRect.origin.x + width,
          info8,
          targetLine.subspan(dstPos.x));
    }
  }
}

void SoftwarePainter::Copy8BitTo8Bit(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) {
  BitmapSizeType width = srcRect.size.x;
  BitmapSizeType height = srcRect.size.y;

  const auto srcPaletteSize = src.GetNumberOfColorsInPalette();
  FixedPalette srcPalette(srcPaletteSize);
  std::array<uint8_t, 256> colorMap{};
  for (size_t i = 0; i < srcPaletteSize; ++i) {
    srcPalette[i] = src.GetColorFromPalette(i);
    colorMap[i] = _palette.findClosestColorIndex(srcPalette[i]);
  }

  std::vector<uint8_t> row_buffer(helpers::BitmapDepth8::BufferBytesPerLine(width));
  // We use srcPalette as target so ReadLineInto will do a direct copy (memcpy)
  const DrawableSurface::TargetInformation info8{DrawableSurface::BitDepth::DEPTH_8, &srcPalette};

  for (BitmapSizeType y = 0; y < height; ++y) {
    src.ReadLineInto(
        srcRect.origin.y + y,
        srcRect.origin.x,
        srcRect.origin.x + width,
        info8,
        row_buffer);

    if (auto targetLine = _target.GetLineSpan(dstPos.y + y);
        !targetLine.empty()) {
      for (BitmapSizeType x = 0; x < width; ++x) {
        targetLine[dstPos.x + x] = colorMap[row_buffer[x]];
      }
    }
  }
}
void SoftwarePainter::DrawGenericData(const DrawableSurface &src, RectT<BitmapSizeType> srcRect, Vec2D<BitmapSizeType> dstPos) {
  // Optimized path for matching 8-bit with palette mapping

  if (helpers::is8Bit(_bit_depth) && helpers::is8Bit(src.GetBitDepth())) {
    if (src.GetBitDepth() == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE || _bit_depth == DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE) {
      Copy8BitNoPalette(src, srcRect, dstPos);
    } else {
      Copy8BitTo8Bit(src, srcRect, dstPos);
    }
    return;
  }

  // Generic Path using 32-bit intermediate
  DrawableSurface::TargetInformation info32;
  info32.bit_depth = DrawableSurface::BitDepth::DEPTH_32;
  info32.shift = helpers::BitmapDepth32::DefaultShift;
  info32.mask = helpers::BitmapDepth32::DefaultMask;

  const BitmapSizeType width = srcRect.size.x;

  std::vector<uint8_t> line32(helpers::BitmapDepth32::BufferBytesPerLine(width));

  for (BitmapSizeType y = 0; y < srcRect.size.y; ++y) {
    src.ReadLineInto(srcRect.origin.y + y, srcRect.origin.x, srcRect.origin.x + width, info32, line32);
    auto targetLine = _target.GetLineSpan(dstPos.y + y);
    if (targetLine.empty()) continue;

    for (BitmapSizeType x = 0; x < width; ++x) {
      Color c = helpers::BitmapDepth32::ReadColor(line32, x, info32.shift, info32.mask);

      const auto dest_x = dstPos.x + x;
      switch (_bit_depth) {
        case DrawableSurface::BitDepth::DEPTH_32: helpers::BitmapDepth32::WriteColor(targetLine, dest_x, c, _target.GetShift(), _target.GetMask()); break;
        case DrawableSurface::BitDepth::DEPTH_16: helpers::BitmapDepth16::WriteColor(targetLine, dest_x, c, _target.GetShift(), _target.GetMask()); break;
        case DrawableSurface::BitDepth::DEPTH_8: helpers::BitmapDepth8::WriteColor(targetLine, dest_x, _palette.findClosestColorIndex(c)); break;
        case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE:
          GetDefaultLogger().Error(source_location::current(), "Cannot draw RGB color to DEPTH_8_NO_PALETTE surface");
          std::abort();
        case DrawableSurface::BitDepth::DEPTH_1: helpers::BitmapDepth1::WriteColor(targetLine, dest_x, _palette.findClosestColorIndex(c)); break;
        default: break;
      }
    }
  }
}

void SoftwarePainter::DrawPoint(const Vec2D<BitmapSizeType> &pos) {
  switch (_penStyle) {
    case PenStyle::NoPen: break;
    case PenStyle::SolidLineColor: PutPixel(pos.x, pos.y, _penColor); break;
    case PenStyle::SolidLineIndex: PutPixel(pos.x, pos.y, _penIndex); break;
  }
}

void SoftwarePainter::DrawRect(const RectT<BitmapSizeType> &rect) {
  if (_brushStyle != BrushStyle::NoBrush) {
    for (BitmapSizeType y = 0; y < rect.size.y; ++y) {
      for (BitmapSizeType x = 0; x < rect.size.x; ++x) {
        if (_brushStyle == BrushStyle::SolidBrushIndex) {
          PutPixel(rect.origin.x + x, rect.origin.y + y, _brushIndex);
        } else {
          PutPixel(rect.origin.x + x, rect.origin.y + y, _brushColor);
        }
      }
    }
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

void SoftwarePainter::DrawEllipse(const RectT<BitmapSizeType> &rect) {
  const long rx = rect.size.x / 2;
  const long ry = rect.size.y / 2;
  const long xc = rect.origin.x + rx;
  const long yc = rect.origin.y + ry;
  long x = 0, y = ry;
  const long rx2 = rx * rx, ry2 = ry * ry;
  long p = e00::lrint(ry2 - rx2 * ry + 0.25 * rx2);
  long dx = 2 * ry2 * x, dy = 2 * rx2 * y;

  auto plot_symmetrical = [&](long px, long py) {
    if (_brushStyle != BrushStyle::NoBrush) {
      for (long ix = xc - px; ix <= xc + px; ++ix) {
        if (_brushStyle == BrushStyle::SolidBrushIndex) {
          PutPixel(ix, yc + py, _brushIndex);
          PutPixel(ix, yc - py, _brushIndex);
        } else {
          PutPixel(ix, yc + py, _brushColor);
          PutPixel(ix, yc - py, _brushColor);
        }
      }
    }

    if (_penStyle != PenStyle::NoPen) {
      DrawPoint(Vec2D<BitmapSizeType>(xc + px, yc + py));
      DrawPoint(Vec2D<BitmapSizeType>(xc - px, yc + py));
      DrawPoint(Vec2D<BitmapSizeType>(xc + px, yc - py));
      DrawPoint(Vec2D<BitmapSizeType>(xc - px, yc - py));
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

void SoftwarePainter::DrawSurface(const DrawableSurface &src,
                                  RectT<BitmapSizeType> srcRect,
                                  Vec2D<BitmapSizeType> dstPos) {
  // Clipping
  if (dstPos.x >= _targetSize.x || dstPos.y >= _targetSize.y) return;

  BitmapSizeType width = srcRect.size.x;
  BitmapSizeType height = srcRect.size.y;

  if (dstPos.x + width > _targetSize.x) width = _targetSize.x - dstPos.x;
  if (dstPos.y + height > _targetSize.y) height = _targetSize.y - dstPos.y;
  if (width == 0 || height == 0) return;

  srcRect.size = {width, height};

  // if (src.Type() == type_id<Bitmap>()) {
  //   auto &bmp = static_cast<const Bitmap &>(src);
  //   if (DrawBitmapData(bmp, srcRect, dstPos)) {
  //     return;
  //   }
  // }
  //
  // if (src.Type() == type_id<Sprite>()) {
  //   auto &spr = static_cast<const Sprite &>(src);
  //   if (DrawSpriteData(spr, srcRect, dstPos)) {
  //     return;
  //   }
  // }

  DrawGenericData(src, srcRect, dstPos);
}

}// namespace e00
