
#pragma once
#include "PlanarSurfaceHw.hpp"

#include <dpmi.h>
#include <pc.h>
#include <sys/nearptr.h>

namespace DOS {
class EGA_Screen : public PlanarSurfaceHw {
  size_t _vram_pool_offset;

public:
  // Initialize standard EGA Mode 10h (640x350, 16 colors)
  // 640 pixels / 8 = 80 bytes per line. Shift = 3.
  EGA_Screen()
      : PlanarSurfaceHw(640, 350, 80, reinterpret_cast<uint8_t *>(0xA0000 + __djgpp_conventional_base), 3),
        _vram_pool_offset(350 * 80)// Off-screen area starts right after line 350 (byte 28000)
  {
    __dpmi_regs r = {};
    r.x.ax = 0x0010;// Set video mode to EGA 640x350 16-color mode
    __dpmi_int(0x10, &r);
  }

  ~EGA_Screen() override {
    __dpmi_regs r = {};
    r.x.ax = 0x0003;// Reset to text mode 3 on exit
    __dpmi_int(0x10, &r);
  }

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<PlanarSurfaceHw>(); }

  // EGA Palette programming requires updating the Attribute Controller registers
  void SetPalette(const e00::FixedPalette &palette) override {
    const size_t max_writes = std::min<size_t>(16, palette.size());

    for (size_t i = 0; i < max_writes; ++i) {
      const e00::Color &c = palette[i];

      // Convert standard 8-bit RGB components down to EGA 2-bit intensity steps
      // EGA color format: rR gG bB (bit 5 = sec R, bit 4 = sec G, bit 3 = sec B, bit 2 = prim R, bit 1 = prim G, bit 0 = prim B)
      uint8_t ega_color = 0;
      if (c.red > 192) ega_color |= (1 << 2) | (1 << 5);// Full bright Red
      else if (c.red > 64)
        ega_color |= (1 << 2);// Dim Red

      if (c.green > 192) ega_color |= (1 << 1) | (1 << 4);// Full bright Green
      else if (c.green > 64)
        ega_color |= (1 << 1);// Dim Green

      if (c.blue > 192) ega_color |= (1 << 0) | (1 << 3);// Full bright Blue
      else if (c.blue > 64)
        ega_color |= (1 << 0);// Dim Blue

      // Update the EGA Attribute Controller palette registers via port 0x3C0
      (void) inportb(0x3DA);     // Reset the 0x3C0 flip-flop toggle to index mode
      outportb(0x3C0, i);        // Select palette register index
      outportb(0x3C0, ega_color);// Write the EGA color code
      outportb(0x3C0, 0x20);     // Re-enable screen output access
    }
  }

  uint8_t *AllocateVRAMSpace(size_t total_bytes) {
    // Standard EGA cards can have less memory. We limit off-screen space to the first 64KB window aperture safely.
    if (_vram_pool_offset + total_bytes <= 65535) {
      uint8_t *addr = _vram_address + _vram_pool_offset;
      _vram_pool_offset += total_bytes;
      return addr;
    }
    return nullptr;// Exceeded allocation bounds, safely falls back to standard RAM
  }
};

}// namespace DOS
