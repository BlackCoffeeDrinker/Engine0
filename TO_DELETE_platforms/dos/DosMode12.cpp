#include "DosMode12.hpp"
#include <go32.h>
#include <dpmi.h>
#include <pc.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>
#include <cstring>
#include <algorithm>

namespace {
constexpr auto VGA_BASE_ADDRESS = 0xA0000;

class ScreenVGA_640X480X4 : public e00::OutputViewport {
  uint8_t _framebuffer[640 * 480]; // 8-bit internal buffer for simplicity, converted to 4-bit planar on flush
  int _old_mode;

  void vsync() {
    while (inportb(0x3DA) & 8);
    while (!(inportb(0x3DA) & 8));
  }

  void write_planar() {
    // Mode 12h is 640x480 16 colors (4 planes)
    // We convert our 8-bit buffer to 4 planes.
    // Plane 0: Blue, Plane 1: Green, Plane 2: Red, Plane 3: Intensity
    
    for (int plane = 0; plane < 4; ++plane) {
        // Select plane
        outportb(0x3C4, 0x02); // Map Mask Register
        outportb(0x3C5, 1 << plane);
        
        for (int y = 0; y < 480; ++y) {
            for (int x8 = 0; x8 < 80; ++x8) {
                uint8_t bits = 0;
                for (int bit = 0; bit < 8; ++bit) {
                    uint8_t pixel = _framebuffer[y * 640 + x8 * 8 + bit];
                    if (pixel & (1 << plane)) {
                        bits |= (0x80 >> bit);
                    }
                }
                _farpokeb(_dos_ds, VGA_BASE_ADDRESS + y * 80 + x8, bits);
            }
        }
    }
  }

public:
  ScreenVGA_640X480X4() : _old_mode(video_get_current_mode()) {
    video_set_current_mode(0x12);
    std::memset(_framebuffer, 0, sizeof(_framebuffer));
  }

  ~ScreenVGA_640X480X4() override {
    video_set_current_mode(_old_mode);
  }

  e00::Vec2<uint16_t> size() const noexcept override {
    return e00::Vec2<uint16_t>(640, 480);
  }

  // Basic drawing could be added here or via a custom painter
};

} // namespace

namespace e00 {
OutputViewport *video_get_screen_mode12() {
    return new ScreenVGA_640X480X4();
}
}
