#include "Mouse.hpp"

#include <Engine/Platform/InputSystem.hpp>

namespace win31 {
namespace {

enum class WinMouseAxis : e00::input_value_t {
  X = 0,
  Y = 1
};

enum class WinMouseButton : e00::input_value_t {
  Left = 10,
  Right = 11,
  Middle = 12
};

class WinMouseSystemImpl : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override { return "Win31 Mouse"; }
  [[nodiscard]] std::string name(e00::input_value_t value) const override {
    switch (value) {
      case 0: return "Mouse X";
      case 1: return "Mouse Y";
      case 10: return "Left Button";
      case 11: return "Right Button";
      case 12: return "Middle Button";
      default: return "Mouse";
    }
  }
};

const e00::InputSystem &WinMouseSystem = WinMouseSystemImpl{};

int g_lastX = 0;
int g_lastY = 0;
bool g_hasPos = false;

}// namespace

void InitMouse() {
  g_hasPos = false;
  g_lastX = 0;
  g_lastY = 0;
}

void QuitMouse() {}

void HandleMouseMessage(e00::Engine &engine, HWND hwnd, UINT msg, WPARAM /*wParam*/, LPARAM lParam) {
  const int x = static_cast<short>(LOWORD(lParam));
  const int y = static_cast<short>(HIWORD(lParam));

  if (g_hasPos) {
    const int dx = x - g_lastX;
    const int dy = y - g_lastY;
    if (dx != 0) {
      e00::InputEvent ev;
      ev.assign_axis(static_cast<e00::input_value_t>(WinMouseAxis::X),
                     static_cast<e00::InputEvent::axis_t>(dx), WinMouseSystem);
      engine.ProcessInputEvent(ev);
    }
    if (dy != 0) {
      e00::InputEvent ev;
      ev.assign_axis(static_cast<e00::input_value_t>(WinMouseAxis::Y),
                     static_cast<e00::InputEvent::axis_t>(dy), WinMouseSystem);
      engine.ProcessInputEvent(ev);
    }
  }

  g_lastX = x;
  g_lastY = y;
  g_hasPos = true;

  auto emitButton = [&](WinMouseButton button, bool down) {
    e00::InputEvent ev;
    ev.assign(down ? e00::InputEvent::Type::KeyDown : e00::InputEvent::Type::KeyUp,
              static_cast<e00::input_value_t>(button), WinMouseSystem);
    engine.ProcessInputEvent(ev);
  };

  switch (msg) {
    case WM_LBUTTONDOWN: emitButton(WinMouseButton::Left, true); break;
    case WM_LBUTTONUP: emitButton(WinMouseButton::Left, false); break;
    case WM_RBUTTONDOWN: emitButton(WinMouseButton::Right, true); break;
    case WM_RBUTTONUP: emitButton(WinMouseButton::Right, false); break;
    case WM_MBUTTONDOWN: emitButton(WinMouseButton::Middle, true); break;
    case WM_MBUTTONUP: emitButton(WinMouseButton::Middle, false); break;
    default: break;
  }

  (void) hwnd;
}

void PollMouse(e00::Engine &engine, HWND hwnd) {
  if (!hwnd) {
    return;
  }

  POINT pt{};
  if (!GetCursorPos(&pt)) {
    return;
  }
  if (!ScreenToClient(hwnd, &pt)) {
    return;
  }

  if (g_hasPos) {
    const int dx = static_cast<int>(pt.x) - g_lastX;
    const int dy = static_cast<int>(pt.y) - g_lastY;
    if (dx != 0) {
      e00::InputEvent ev;
      ev.assign_axis(static_cast<e00::input_value_t>(WinMouseAxis::X),
                     static_cast<e00::InputEvent::axis_t>(dx), WinMouseSystem);
      engine.ProcessInputEvent(ev);
    }
    if (dy != 0) {
      e00::InputEvent ev;
      ev.assign_axis(static_cast<e00::input_value_t>(WinMouseAxis::Y),
                     static_cast<e00::InputEvent::axis_t>(dy), WinMouseSystem);
      engine.ProcessInputEvent(ev);
    }
  }

  g_lastX = static_cast<int>(pt.x);
  g_lastY = static_cast<int>(pt.y);
  g_hasPos = true;
}

}// namespace win31
