
#include "mouse.h"

#include <chrono>
#include <cstring>
#include <dpmi.h>

namespace {
bool _has_mouse = false;

int last_x = 0;
int last_y = 0;
int last_buttons = 0;

// Mouse sensitivity (mickeys per pixel), queried from INT 33h function 0x1B
float mickeys_per_hpixel;// horizontal mickeys per pixel (default: 8)
float mickeys_per_vpixel;// vertical mickeys per pixel (default: 16)

enum class DosMouseAxis : e00::input_value_t {
  X = 0,
  Y = 1
};

enum class DosMouseButton : e00::input_value_t {
  Left = 10,
  Right = 11,
  Middle = 12
};

class DosMouseInputSystemImpl : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override { return "DOS Mouse"; }
  [[nodiscard]] std::string name(e00::input_value_t value) const override {
    switch (value) {
      case 0:
        return "Mouse X";
      case 1:
        return "Mouse Y";
      case 10:
        return "Left Button";
      case 11:
        return "Right Button";
      case 12:
        return "Middle Button";
      default:
        return "Unknown Mouse Event";
    }
  }
};

const e00::InputSystem &DosMouseInputSystem = DosMouseInputSystemImpl{};

}// namespace

bool InitMouse() {
  __dpmi_regs regs = {};
  regs.x.ax = 0;
  __dpmi_int(0x33, &regs);
  if (regs.x.ax == 0) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Mouse: INT 33h AX=0 failed, no mouse driver found");
    return false;// no mouse found
  }

  // How many buttons?
  if (regs.x.bx == 0) {
    e00::GetDefaultLogger().Info(e00::source_location::current(), "Mouse: bx = 0 (other than two)");
  } else if (regs.x.bx == 3) {
    e00::GetDefaultLogger().Info(e00::source_location::current(), "Mouse: bx = 3 (Mouse Systems mouse)");
  } else {
    e00::GetDefaultLogger().Info(e00::source_location::current(), "Mouse: bx = {} (number of buttons)", regs.x.bx);
  }

  std::memset(&regs, 0, sizeof(regs));
  regs.x.ax = 0x1B;// Get Mouse Sensitivity
  __dpmi_int(0x33, &regs);
  // Default values: 8 mickeys per 8 pixels (horiz), 16 mickeys per 8 pixels (vert)
  // which means 1.0 and 2.0 mickeys per pixel.
  if (regs.x.bx > 0 && regs.x.cx > 0) {
    mickeys_per_hpixel = static_cast<float>(regs.x.bx);
    mickeys_per_vpixel = static_cast<float>(regs.x.cx);
  } else {
    mickeys_per_hpixel = 8.0f;
    mickeys_per_vpixel = 16.0f;
  }

  // We want mickeys per pixel, so divide by 8
  mickeys_per_hpixel /= 8.0f;
  mickeys_per_vpixel /= 8.0f;

  e00::GetDefaultLogger().Info(e00::source_location::current(), "Mouse initialized. Mickeys/pixel: H={:.2f}, V={:.2f}", mickeys_per_hpixel, mickeys_per_vpixel);

  // Hide cursor (AX=2). This sometimes helps the driver to accumulate mickeys
  // in AX=B instead of consuming them for internal cursor movement.
  std::memset(&regs, 0, sizeof(regs));
  regs.x.ax = 0x0002;
  __dpmi_int(0x33, &regs);

  // Initialize last positions to current values
  std::memset(&regs, 0, sizeof(regs));
  regs.x.ax = 0x0003;
  __dpmi_int(0x33, &regs);
  last_x = static_cast<int16_t>(regs.x.cx);
  last_y = static_cast<int16_t>(regs.x.dx);
  last_buttons = regs.x.bx;

  _has_mouse = true;
  return true;
}

void QuitMouse() {
}

void SendMouseEvent(e00::Engine &engine) {
  if (!_has_mouse) {
    return;
  }

  __dpmi_regs regs = {};

  // 1. Read motion counters (mickeys). This is the primary movement data.
  regs.x.ax = 0x000B;
  __dpmi_int(0x33, &regs);
  auto raw_mickeys_x = static_cast<int16_t>(regs.x.cx);
  auto raw_mickeys_y = static_cast<int16_t>(regs.x.dx);

  // 2. Read buttons. Under Win95, ignore CX/DX absolute coordinates for
  // delta tracking to prevent register clearing race conditions.
  std::memset(&regs, 0, sizeof(regs));
  regs.x.ax = 0x0003;
  __dpmi_int(0x33, &regs);
  const uint16_t buttons = regs.x.bx;

  // FIX: Process button states independently of cursor coordinates
  // to ensure stationary clicks are never dropped.
  if (buttons != last_buttons) {
    e00::GetDefaultLogger().Info(e00::source_location::current(), "Mouse: buttons changed = {}", buttons);

    for (int i = 0; i < 3; ++i) {
      bool current = (buttons & (1 << i));
      bool last = (last_buttons & (1 << i));
      if (current != last) {
        e00::InputEvent event;
        event.assign(current ? e00::InputEvent::Type::KeyDown : e00::InputEvent::Type::KeyUp,
                     static_cast<e00::input_value_t>(10 + i),
                     DosMouseInputSystem);
        engine.ProcessInputEvent(event);
      }
    }
    last_buttons = buttons;
  }

  // 3. Compute pixel movement deltas exclusively via mickeys
  float pixel_delta_x = static_cast<float>(raw_mickeys_x) / mickeys_per_hpixel;
  float pixel_delta_y = static_cast<float>(raw_mickeys_y) / mickeys_per_vpixel;

  // Accumulate fractional pixels
  static float acc_x = 0;
  static float acc_y = 0;

  acc_x += pixel_delta_x;
  acc_y += pixel_delta_y;

  const auto out_x = static_cast<int16_t>(acc_x);
  const auto out_y = static_cast<int16_t>(acc_y);

  acc_x -= static_cast<float>(out_x);
  acc_y -= static_cast<float>(out_y);

  if (out_x != 0 || out_y != 0) {
    e00::GetDefaultLogger().Info(
        e00::source_location::current(),
        "Mouse: dx = {}, dy = {} | raw_mickeys = {}, {}",
        out_x, out_y,
        raw_mickeys_x, raw_mickeys_y);

    // Send motion events to replay engine
    if (out_x != 0) {
      e00::InputEvent event;
      event.assign_axis(static_cast<e00::input_value_t>(DosMouseAxis::X), out_x, DosMouseInputSystem);
      engine.ProcessInputEvent(event);
    }
    if (out_y != 0) {
      e00::InputEvent event;
      event.assign_axis(static_cast<e00::input_value_t>(DosMouseAxis::Y), out_y, DosMouseInputSystem);
      engine.ProcessInputEvent(event);
    }
  }
}
