
#pragma once

#include "PlanarSurfaceHw.hpp"

#include <dpmi.h>
#include <pc.h>
#include <sys/nearptr.h>

namespace DOS {
class VGA_12h : public PlanarSurfaceHw {
  size_t _vram_pool_offset;// Allocator floor tracking variable

public:
  VGA_12h()
      : PlanarSurfaceHw(640, 480, 80, reinterpret_cast<uint8_t *>(0xA0000 + __djgpp_conventional_base), 3),
        _vram_pool_offset(480 * 80)// Off-screen allocations start right after screen memory (byte 38400)
  {
    // Change the hardware state to Mode 12h via Real-Mode Interrupt
    __dpmi_regs r = {};
    r.x.ax = 0x0012;
    __dpmi_int(0x10, &r);
  }

  ~VGA_12h() override {
    // Return system back to standard text mode 3 on destruction
    __dpmi_regs r = {};
    r.x.ax = 0x0003;
    __dpmi_int(0x10, &r);
  }

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<PlanarSurfaceHw>(); }

  // Allocation helper called by the namespace platform::CreateSurface function
  uint8_t *AllocateVRAMSpace(size_t total_bytes) {
    if (_vram_pool_offset + total_bytes <= 65535) {
      uint8_t *addr = _vram_address + _vram_pool_offset;
      _vram_pool_offset += total_bytes;
      return addr;
    }
    return nullptr;// Out of high-speed hardware VRAM space
  }

  [[nodiscard]] std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) override {
    const size_t bpl = (size.x + 7) / 8;
    const size_t total_bytes = bpl * size.y;
    uint8_t *allocated_addr = nullptr;

    if (where == platform::MemoryPlacement::VideoMemOnly) {
      allocated_addr = AllocateVRAMSpace(total_bytes);
      if (!allocated_addr) return nullptr;
    }

    return std::make_unique<PlanarSurfaceHw>(static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y), bpl, allocated_addr, 3);
  }
};

}// namespace DOS
