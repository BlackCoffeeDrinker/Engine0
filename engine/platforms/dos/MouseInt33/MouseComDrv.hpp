#pragma once

#include "Engine.hpp"

const e00::InputSystem &MouseInputSystem() noexcept;

class MouseComDrv {
  bool _has_mouse;
  bool _has_mouse_wheel;
  int _wheel_pos;

  e00::helper::input::AxisHelper<0> _axis_x{ MouseInputSystem() };
  e00::helper::input::AxisHelper<1> _axis_y{ MouseInputSystem() };

  e00::helper::input::ButtonHelper<0> _left_btn{ MouseInputSystem() };
  e00::helper::input::ButtonHelper<1> _right_btn{ MouseInputSystem() };
  e00::helper::input::ButtonHelper<2> _middle_btn{ MouseInputSystem() };

public:
  MouseComDrv();
  ~MouseComDrv();

  [[nodiscard]] bool has_mouse() const { return _has_mouse; }

  void mouse_update();

  e00::InputEvent event_data() noexcept {
    if (_axis_x) {
      return _axis_x.GetAndReset();
    }

    if (_axis_y) {
      return _axis_y.GetAndReset();
    }

    if (_left_btn) {
      return _left_btn.GetAndReset();
    }

    if (_right_btn) {
      return _right_btn.GetAndReset();
    }

    if (_middle_btn) {
      return _middle_btn.GetAndReset();
    }

    return {};
  }
};
