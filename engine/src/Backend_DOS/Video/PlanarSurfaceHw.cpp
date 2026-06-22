
#include "PlanarPainter.hpp"
#include "PlanarSurfaceHw.hpp"

#include <Engine/DefaultBitmapHelpers.hpp>
#include <Engine/Resource/Bitmap.hpp>

#include <algorithm>
#include <cstring>
#include <pc.h>

namespace DOS {
void PlanarSurfaceHw::ReadLineInto(e00::BitmapSizeType line,
                                     e00::BitmapSizeType startX, e00::BitmapSizeType endX,
                                     const TargetInformation &targetInformation,
                                     std::span<uint8_t> targetBuffer) const {

  // Safety bounds clipping check
  if (line >= _height || startX >= endX) return;

  const size_t run_width = std::min<size_t>(endX - startX, (startX < _width) ? (_width - startX) : 0);
  const size_t bytesPerPixel = (e00::DepthEnumToBits(targetInformation.bit_depth) + 7) / 8;

  // If the asset is a system RAM fallback, unpack from the planar RAM buffer
  if (!IsHardwareAccelerated()) {
    const size_t plane_size = _bytes_per_line * _height;
    const uint8_t *p0_row = _ram_fallback.data() + (0 * plane_size) + (line * _bytes_per_line);
    const uint8_t *p1_row = _ram_fallback.data() + (1 * plane_size) + (line * _bytes_per_line);
    const uint8_t *p2_row = _ram_fallback.data() + (2 * plane_size) + (line * _bytes_per_line);
    const uint8_t *p3_row = _ram_fallback.data() + (3 * plane_size) + (line * _bytes_per_line);

    size_t out_pixel_count = 0;
    const size_t start_byte = std::min(_bytes_per_line, (size_t)(startX >> _pixels_per_byte_shift));
    const size_t end_byte = std::min(_bytes_per_line, (size_t)((endX + (1 << _pixels_per_byte_shift) - 1) >> _pixels_per_byte_shift));

    for (size_t b = start_byte; b < end_byte; ++b) {
      uint8_t p0 = p0_row[b], p1 = p1_row[b], p2 = p2_row[b], p3 = p3_row[b];

      if (_pixels_per_byte_shift == 3) {
        for (int bit = 7; bit >= 0; --bit) {
          size_t absolute_x = (b * 8) + (7 - bit);
          if (absolute_x < startX || absolute_x >= endX || out_pixel_count >= run_width) continue;

          uint8_t color_index = (((p0 >> bit) & 1) << 0) |
                                (((p1 >> bit) & 1) << 1) |
                                (((p2 >> bit) & 1) << 2) |
                                (((p3 >> bit) & 1) << 3);

          if (targetInformation.bit_depth == BitDepth::DEPTH_8) {
            targetBuffer[out_pixel_count * bytesPerPixel] = color_index;
          } else if (targetInformation.bit_depth == BitDepth::DEPTH_16) {
            const auto &c = GetColorFromPalette(color_index);
            e00::helpers::BitmapDepth16::WriteColor(targetBuffer, out_pixel_count, c, targetInformation.shift, targetInformation.mask);
          }
          out_pixel_count++;
        }
      } else {
        uint8_t indices[4] = {p0, p1, p2, p3};
        for (int i = 0; i < 4; ++i) {
          size_t absolute_x = (b * 4) + i;
          if (absolute_x < startX || absolute_x >= endX || out_pixel_count >= run_width) continue;
          targetBuffer[out_pixel_count * bytesPerPixel] = indices[i];
          out_pixel_count++;
        }
      }
    }
    return;
  }

  // --- HARDWARE PLANAR EXTRACTION (EGA / VGA MODE 12h) ---
  // Save the current graphics controller read mode context state (Port 0x3CE index 5)
  outportb(0x3CE, 0x05);
  const uint8_t original_mode_reg = inportb(0x3CF);

  // Put the hardware into Read Mode 0 to harvest planes sequentially
  outportb(0x3CF, original_mode_reg & ~0x08);

  const uint8_t *vram_row_start = _vram_address + (line * _bytes_per_line);
  size_t out_pixel_count = 0;

  // Calculate byte boundary alignments for column spans inside VRAM rows
  const size_t start_byte = std::min(_bytes_per_line, (size_t)(startX >> _pixels_per_byte_shift));
  const size_t end_byte = std::min(_bytes_per_line, (size_t)((endX + (1 << _pixels_per_byte_shift) - 1) >> _pixels_per_byte_shift));

  for (size_t b = start_byte; b < end_byte; ++b) {
    uint8_t p0 = 0, p1 = 0, p2 = 0, p3 = 0;

    // Extract Plane 0
    outportb(0x3CE, 0x04);
    outportb(0x3CF, 0);
    p0 = vram_row_start[b];
    // Extract Plane 1
    outportb(0x3CE, 0x04);
    outportb(0x3CF, 1);
    p1 = vram_row_start[b];
    // Extract Plane 2
    outportb(0x3CE, 0x04);
    outportb(0x3CF, 2);
    p2 = vram_row_start[b];
    // Extract Plane 3
    outportb(0x3CE, 0x04);
    outportb(0x3CF, 3);
    p3 = vram_row_start[b];

    if (_pixels_per_byte_shift == 3) {
      // Unpack 8 pixels out of this planar byte sequence
      for (int bit = 7; bit >= 0; --bit) {
        size_t absolute_x = (b * 8) + (7 - bit);
        if (absolute_x < startX || absolute_x >= endX || out_pixel_count >= run_width) continue;

        uint8_t color_index = (((p0 >> bit) & 1) << 0) |
                              (((p1 >> bit) & 1) << 1) |
                              (((p2 >> bit) & 1) << 2) |
                              (((p3 >> bit) & 1) << 3);

        // Transcode extracted value directly into output targetBuffer array slice
        if (targetInformation.bit_depth == BitDepth::DEPTH_8) {
          targetBuffer[out_pixel_count * bytesPerPixel] = color_index;
        } else if (targetInformation.bit_depth == BitDepth::DEPTH_16) {
          // Re-pack palette index colors to match high-color bits if called by a 16-bit BMP exporter
          const auto &c = GetColorFromPalette(color_index);
          e00::helpers::BitmapDepth16::WriteColor(targetBuffer, out_pixel_count, c, targetInformation.shift, targetInformation.mask);
        }
        out_pixel_count++;
      }
    } else {
      // Mode X/Y: Unpack 4 interleaved pixel indices
      uint8_t indices[4] = {p0, p1, p2, p3};
      for (int i = 0; i < 4; ++i) {
        size_t absolute_x = (b * 4) + i;
        if (absolute_x < startX || absolute_x >= endX || out_pixel_count >= run_width) continue;

        targetBuffer[out_pixel_count * bytesPerPixel] = indices[i];
        out_pixel_count++;
      }
    }
  }

  // Restore original read configuration settings
  outportb(0x3CE, 0x05);
  outportb(0x3CF, original_mode_reg);
}

std::unique_ptr<e00::Painter> PlanarSurfaceHw::BeginDraw() {
  uint8_t *addr = _vram_address;
  if (addr == nullptr) addr = _ram_fallback.data();

  return std::make_unique<PlanarPainter>(
      *this,
      _width,
      _height,
      _bytes_per_line,
      addr,
      _pixels_per_byte_shift,
      _palette);
}

void PlanarSurfaceHw::SetPalette(const e00::FixedPalette &palette) {
  // Mode 12h (16-color) natively caps palette writes.
  // Mode X (256-color planar) allows 256.
  const size_t limit = (_pixels_per_byte_shift == 3) ? 16 : 256;
  const size_t max_writes = std::min<size_t>(limit, palette.size());

  if (IsHardwareAccelerated() && _pixels_per_byte_shift == 3) {
    // FIX: VGA Attribute Controller mapping for 16-color modes (e.g., Mode 12h).
    // Force direct 1:1 mapping (index 0-15 -> DAC 0-15).
    (void)inportb(0x3DA); // Reset flip-flop
    for (uint8_t i = 0; i < 16; ++i) {
      outportb(0x3C0, i); // Index
      outportb(0x3C0, i); // Data
    }
    (void)inportb(0x3DA); // Reset again before enabling
    outportb(0x3C0, 0x20); // Enable display
  }

  if (IsHardwareAccelerated()) {
    for (size_t i = 0; i < max_writes; ++i) {
      const auto &newColor = palette[i];
      // Always update hardware for safety, but still track in local _palette
      outportb(0x3C8, i);
      outportb(0x3C9, newColor.red >> 2);
      outportb(0x3C9, newColor.green >> 2);
      outportb(0x3C9, newColor.blue >> 2);
      
      _palette.set(i, newColor);
    }
  } else {
    // Non-hardware: just update local state
    for (size_t i = 0; i < max_writes; ++i) {
      _palette.set(i, palette[i]);
    }
  }
}

std::unique_ptr<e00::DrawableSurface> PlanarSurfaceHw::CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) {
  size_t bpl = (size.x + (1 << _pixels_per_byte_shift) - 1) >> _pixels_per_byte_shift;
  auto surface = std::make_unique<PlanarSurfaceHw>(
      static_cast<uint16_t>(size.x),
      static_cast<uint16_t>(size.y),
      bpl,
      nullptr,
      _pixels_per_byte_shift);
  surface->SetPalette(_palette);
  return surface;
}

}// namespace DOS
