#include "MouseComDrv.hpp"

#include <dpmi.h>

constexpr static int MOUSE_DRV_INTR = 0x33;
constexpr static int MOUSE_DRV_FUNC_RESET = 0;
constexpr static int MOUSE_DRV_FUN_HIDE_CURSOR = 0x02;
constexpr static int MOUSE_DRV_FUNC_GET_POS_AND_BTNS = 0x3;
constexpr static int MOUSE_DRV_FUNC_GET_BUTTON_PRESS = 0x5;
constexpr static int MOUSE_DRV_FUNC_GET_COUNTERS = 0xB;

namespace {
class MouseInputSystem : public e00::InputSystem {
public:
  std::string name() const override {
    return "mouse";
  }
  std::string name(uint16_t value) const override {
    return "some value";
  }
};

const MouseInputSystem mouse_input_system;
}// namespace

const e00::InputSystem &MouseInputSystem() noexcept {
  return mouse_input_system;
}

MouseComDrv::MouseComDrv() : _has_mouse(false),
                             _has_mouse_wheel(false),
                             _wheel_pos(0) {

  // Detect mouse driver and mouse existence
  {
    __dpmi_regs regs;
    regs.x.ax = MOUSE_DRV_FUNC_RESET;
    __dpmi_int(MOUSE_DRV_INTR, &regs);
    _has_mouse = regs.x.ax != 0;
    if (_has_mouse) {
      switch (regs.x.bx) {
        case 0x2:
        case 0xFFFFU:// Two buttons
          break;
        case 0x3:// Three buttons
          break;
      }
    }
  }

  // detect CuteMouse 2.0+ wheel api
  if (_has_mouse) {
    __dpmi_regs regs;
    regs.x.ax = 0x11;
    __dpmi_int(MOUSE_DRV_INTR, &regs);
    _has_mouse_wheel = ((regs.x.ax == 0x574D) && (regs.x.cx & 1));
  }

  // Disable the mouse cursor
  if (_has_mouse) {
    __dpmi_regs regs;
    regs.x.ax = MOUSE_DRV_FUN_HIDE_CURSOR;
    __dpmi_int(MOUSE_DRV_INTR, &regs);
  }
}

void MouseComDrv::mouse_update() {
  if (!_has_mouse) {
    return;
  }

  __dpmi_regs regs;
  regs.x.ax = MOUSE_DRV_FUNC_GET_COUNTERS;
  regs.x.bx = 0;
  regs.x.cx = 0;
  regs.x.dx = 0;
  __dpmi_int(MOUSE_DRV_INTR, &regs);
  _axis_x.add((short)regs.x.cx);
  _axis_y.add((short)regs.x.dx);

  regs.x.ax = MOUSE_DRV_FUNC_GET_POS_AND_BTNS;
  regs.x.bx = 0;
  regs.x.cx = 0;
  regs.x.dx = 0;
  __dpmi_int(MOUSE_DRV_INTR, &regs);

  if ((regs.x.bx & 0x01U) > 0) {
    _left_btn.set(e00::InputEvent::Type::Press);
  }

  if ((regs.x.bx & 0x02U) > 0) {
    _right_btn.set(e00::InputEvent::Type::Press);
  }

  if ((regs.x.bx & 0x03U) > 0) {
    _middle_btn.set(e00::InputEvent::Type::Press);
  }
}

MouseComDrv::~MouseComDrv() = default;
