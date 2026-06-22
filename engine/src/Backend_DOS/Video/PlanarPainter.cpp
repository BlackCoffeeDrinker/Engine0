#include "PlanarPainter.hpp"

#include "PlanarSurfaceHw.hpp"
#include <cstring>

void DOS::PlanarPainter::write_pixel_planar(int x, int y, uint8_t index) {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return;

  const int byte_offset = x >> _shift;
  const int bit_index = x % (1 << _shift);

  if (_is_hardware) {
    uint8_t *pixel_addr = _vram_base + (y * _pitch) + byte_offset;
    outportb(GRAPHICS_MODE_REGISTERS, 0x08);
    outportb(0x3CF, 0x80 >> bit_index);

    const volatile uint8_t latch_prime = *pixel_addr;
    (void)latch_prime;

    *pixel_addr = index & 0x0F;
  } else {
    // Software Planar RAM fallback
    const uint8_t bit = 0x80 >> bit_index;
    const size_t plane_size = _pitch * _height;
    const size_t off = (y * _pitch) + byte_offset;

    for (int p = 0; p < 4; ++p) {
      uint8_t *plane_ptr = _vram_base + (p * plane_size) + off;
      if (index & (1 << p)) {
        *plane_ptr |= bit;
      } else {
        *plane_ptr &= ~bit;
      }
    }
  }
}

void DOS::PlanarPainter::DrawPoint(const e00::Vec2D<e00::BitmapSizeType> &pos) {
  if (_penStyle == PenStyle::NoPen) return;
  write_pixel_planar(pos.x, pos.y, _penIndex);
}

void DOS::PlanarPainter::DrawLine(const e00::Vec2D<e00::BitmapSizeType> &start, const e00::Vec2D<e00::BitmapSizeType> &end) {
  if (_penStyle == PenStyle::NoPen) return;

  int x0 = start.x;
  int y0 = start.y;
  const int x1 = end.x;
  const int y1 = end.y;

  const int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  const int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  for (;;) {
    write_pixel_planar(x0, y0, _penIndex);
    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
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

void DOS::PlanarPainter::DrawRect(const e00::RectT<e00::BitmapSizeType> &rect) {
  const int start_x = std::max<int>(0, rect.origin.x);
  const int end_x = std::min<int>(_width, rect.origin.x + rect.size.x);
  const int start_y = std::max<int>(0, rect.origin.y);
  const int end_y = std::min<int>(_height, rect.origin.y + rect.size.y);

  if (start_x >= end_x || start_y >= end_y) return;

  // 1. Process Solid Interior Fill Phase
  if (_brushStyle != BrushStyle::NoBrush) {
    if (_is_hardware) {
      outportb(GRAPHICS_MODE_REGISTERS, 0x08); // Select Bit Mask register

      for (int y = start_y; y < end_y; ++y) {
        uint8_t *vram_row = _vram_base + (y * _pitch);
        int x = start_x;

        // Left fractional byte
        if (x < end_x && (x & 7) != 0) {
          const int bit_start = x & 7;
          const int count = std::min<int>(8 - bit_start, end_x - x);
          uint8_t mask = 0;
          for (int i = 0; i < count; ++i) mask |= (0x80 >> (bit_start + i));

          outportb(0x3CF, mask);
          const volatile uint8_t dummy = vram_row[x >> 3];
          (void)dummy;
          vram_row[x >> 3] = _brushIndex;
          x += count;
        }

        // Middle full bytes
        if (x + 8 <= end_x) {
          const int bytes = (end_x - x) >> 3;
          outportb(0x3CF, 0xFF);
          std::memset(vram_row + (x >> 3), _brushIndex, bytes);
          x += (bytes << 3);
        }

        // Right fractional byte
        if (x < end_x) {
          const int count = end_x - x;
          uint8_t mask = 0;
          for (int i = 0; i < count; ++i) mask |= (0x80 >> i);

          outportb(0x3CF, mask);
          const volatile uint8_t dummy = vram_row[x >> 3];
          (void)dummy;
          vram_row[x >> 3] = _brushIndex;
        }
      }
    } else {
      // Software Planar RAM Fill
      const size_t plane_size = _pitch * _height;
      for (int y = start_y; y < end_y; ++y) {
        int x = start_x;
        const size_t row_off = y * _pitch;

        while (x < end_x) {
          int bit_start = x & 7;
          int count = std::min<int>(8 - bit_start, end_x - x);
          uint8_t mask = 0;
          for (int i = 0; i < count; ++i) mask |= (0x80 >> (bit_start + i));

          const size_t off = row_off + (x >> 3);
          for (int p = 0; p < 4; ++p) {
            uint8_t *ptr = _vram_base + (p * plane_size) + off;
            if (_brushIndex & (1 << p)) {
              *ptr |= mask;
            } else {
              *ptr &= ~mask;
            }
          }
          x += count;
        }
      }
    }
  }

  // 2. Process Outline Pen Bounds Phase
  if (_penStyle != PenStyle::NoPen) {
    DrawLine({rect.origin.x, rect.origin.y}, {rect.origin.x + rect.size.x - 1, rect.origin.y});
    DrawLine({rect.origin.x, rect.origin.y + rect.size.y - 1}, {rect.origin.x + rect.size.x - 1, rect.origin.y + rect.size.y - 1});
    DrawLine({rect.origin.x, rect.origin.y}, {rect.origin.x, rect.origin.y + rect.size.y - 1});
    DrawLine({rect.origin.x + rect.size.x - 1, rect.origin.y}, {rect.origin.x + rect.size.x - 1, rect.origin.y + rect.size.y - 1});
  }
}

void DOS::PlanarPainter::DrawEllipse(const e00::RectT<e00::BitmapSizeType> &rect) {
  // Outline processing maps directly to midpoint configuration arrays
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
      // Fill horizontal scanning spans between interior boundaries
      for (long ix = xc - px; ix <= xc + px; ++ix) write_pixel_planar(ix, yc + py, _brushIndex);
      for (long ix = xc - px; ix <= xc + px; ++ix) write_pixel_planar(ix, yc - py, _brushIndex);
    }
    if (_penStyle != PenStyle::NoPen) {
      write_pixel_planar(xc + px, yc + py, _penIndex);
      write_pixel_planar(xc - px, yc + py, _penIndex);
      write_pixel_planar(xc + px, yc - py, _penIndex);
      write_pixel_planar(xc - px, yc - py, _penIndex);
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

void DOS::PlanarPainter::DrawSurface(const e00::DrawableSurface &src, e00::RectT<e00::BitmapSizeType> srcRect, e00::Vec2D<e00::BitmapSizeType> dstPos) {
  const int start_y = std::max<int>(0, dstPos.y);
  const int end_y = std::min<int>(_height, dstPos.y + srcRect.size.y);
  const int start_x = std::max<int>(0, dstPos.x);
  const int end_x = std::min<int>(_width, dstPos.x + srcRect.size.x);

  if (start_x >= end_x || start_y >= end_y) return;

  const int clip_width = end_x - start_x;
  const int clip_src_x = srcRect.origin.x + (start_x - dstPos.x);

  // 1. OPTIMIZED PLANAR-TO-PLANAR PATH (The "4 memcpy" goal)
  // Only if aligned and width is multiple of 8 to avoid complex edge cases with memcpy
  if (src.Type() == e00::type_id<PlanarSurfaceHw>() && (start_x & 7) == 0 && (clip_src_x & 7) == 0 && (clip_width & 7) == 0) {
    const auto &planar_src = static_cast<const PlanarSurfaceHw &>(src);
    const int num_bytes_x = clip_width >> 3;
    const int dst_byte_x = start_x >> 3;
    const int src_byte_x = clip_src_x >> 3;

    if (_is_hardware) {
      outportb(GRAPHICS_MODE_REGISTERS, 0x05);
      outportb(0x3CF, 0x00); // Write Mode 0

      for (int plane = 0; plane < 4; ++plane) {
        outportb(0x3C4, 0x02); // Map Mask
        outportb(0x3C5, 1 << plane);

        const uint8_t *src_plane = planar_src.GetPlaneAddress(plane);
        for (int y = start_y; y < end_y; ++y) {
          const int src_y = srcRect.origin.y + (y - dstPos.y);
          uint8_t *dst_ptr = _vram_base + (y * _pitch) + dst_byte_x;
          const uint8_t *src_ptr = src_plane + (src_y * planar_src.GetBytesPerLine()) + src_byte_x;
          std::memcpy(dst_ptr, src_ptr, num_bytes_x);
        }
      }
      outportb(GRAPHICS_MODE_REGISTERS, 0x05);
      outportb(0x3CF, 0x02); // Back to Write Mode 2
    } else {
      const size_t dst_plane_size = _pitch * _height;
      for (int plane = 0; plane < 4; ++plane) {
        uint8_t *dst_plane = _vram_base + (plane * dst_plane_size);
        const uint8_t *src_plane = planar_src.GetPlaneAddress(plane);
        for (int y = start_y; y < end_y; ++y) {
          const int src_y = srcRect.origin.y + (y - dstPos.y);
          uint8_t *dst_ptr = dst_plane + (y * _pitch) + dst_byte_x;
          const uint8_t *src_ptr = src_plane + (src_y * planar_src.GetBytesPerLine()) + src_byte_x;
          std::memcpy(dst_ptr, src_ptr, num_bytes_x);
        }
      }
    }
    return;
  }

  // 2. CHUNKY-TO-PLANAR PATH (Process one plane at a time)
  std::vector<uint8_t> rowBuffer(clip_width);
  std::vector<uint8_t> planeRow( (clip_width + 7) / 8 );
  e00::DrawableSurface::TargetInformation targetInfo{};
  targetInfo.bit_depth = e00::DrawableSurface::BitDepth::DEPTH_8;
  targetInfo.palette = &_palette;

  if (_is_hardware) {
    outportb(GRAPHICS_MODE_REGISTERS, 0x05);
    outportb(0x3CF, 0x00); // Write Mode 0
    outportb(GRAPHICS_MODE_REGISTERS, 0x08);
    outportb(0x3CF, 0xFF); // Bit Mask all pixels

    for (int plane = 0; plane < 4; ++plane) {
      outportb(0x3C4, 0x02); // Map Mask
      outportb(0x3C5, 1 << plane);

      for (int y = start_y; y < end_y; ++y) {
        const int src_y = srcRect.origin.y + (y - dstPos.y);
        src.ReadLineInto(src_y, clip_src_x, clip_src_x + clip_width, targetInfo, rowBuffer);

        if ((start_x & 7) == 0 && (clip_width & 7) == 0) {
          // Fast bit packing
          std::memset(planeRow.data(), 0, planeRow.size());
          for (int i = 0; i < clip_width; ++i) {
            if (rowBuffer[i] & (1 << plane)) planeRow[i >> 3] |= (0x80 >> (i & 7));
          }
          std::memcpy(_vram_base + (y * _pitch) + (start_x >> 3), planeRow.data(), planeRow.size());
        } else {
          // Pixel by pixel (safe against non-alignment)
          uint8_t *dst_row = _vram_base + (y * _pitch);
          for (int i = 0; i < clip_width; ++i) {
            const int dx = start_x + i;
            const uint8_t mask = 0x80 >> (dx & 7);
            outportb(0x3CF, mask);
            const volatile uint8_t dummy = dst_row[dx >> 3]; (void)dummy;
            dst_row[dx >> 3] = (rowBuffer[i] & (1 << plane)) ? 0xFF : 0x00;
          }
          outportb(0x3CF, 0xFF); // Reset bit mask
        }
      }
    }
    outportb(GRAPHICS_MODE_REGISTERS, 0x05);
    outportb(0x3CF, 0x02); // Back to Write Mode 2
  } else {
    // RAM Destination
    const size_t plane_size = _pitch * _height;
    for (int plane = 0; plane < 4; ++plane) {
      uint8_t *dst_plane = _vram_base + (plane * plane_size);
      for (int y = start_y; y < end_y; ++y) {
        const int src_y = srcRect.origin.y + (y - dstPos.y);
        src.ReadLineInto(src_y, clip_src_x, clip_src_x + clip_width, targetInfo, rowBuffer);

        for (int i = 0; i < clip_width; ++i) {
          const int dx = start_x + i;
          uint8_t *ptr = dst_plane + (y * _pitch) + (dx >> 3);
          uint8_t bit = 0x80 >> (dx & 7);
          if (rowBuffer[i] & (1 << plane)) *ptr |= bit;
          else *ptr &= ~bit;
        }
      }
    }
  }
}
