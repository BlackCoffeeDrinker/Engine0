#include "Keyboard.hpp"

#include <Engine/Platform/InputSystem.hpp>

namespace win31 {
namespace {

enum class WinKey : e00::input_value_t {
  Unknown = 0,
  Escape = 1,
  Enter = 2,
  Space = 3,
  Left = 4,
  Right = 5,
  Up = 6,
  Down = 7,
  Tab = 8,
  Backspace = 9,
  Shift = 10,
  Control = 11,
  Alt = 12,
  A = 20,
  // ... letters use 'A'+offset via raw VK when in A-Z / 0-9 range
};

class WinKeyboardSystemImpl : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override { return "Win31 Keyboard"; }
  [[nodiscard]] std::string name(e00::input_value_t value) const override {
    switch (static_cast<WinKey>(value)) {
      case WinKey::Escape: return "Escape";
      case WinKey::Enter: return "Enter";
      case WinKey::Space: return "Space";
      case WinKey::Left: return "Left";
      case WinKey::Right: return "Right";
      case WinKey::Up: return "Up";
      case WinKey::Down: return "Down";
      case WinKey::Tab: return "Tab";
      case WinKey::Backspace: return "Backspace";
      case WinKey::Shift: return "Shift";
      case WinKey::Control: return "Control";
      case WinKey::Alt: return "Alt";
      default:
        if (value >= 20 && value < 20 + 26) {
          char buf[2] = {static_cast<char>('A' + (value - 20)), 0};
          return buf;
        }
        return "Key";
    }
  }
};

const e00::InputSystem &WinKeyboardSystem = WinKeyboardSystemImpl{};

e00::input_value_t MapVirtualKey(WPARAM vk) {
  switch (static_cast<int>(vk)) {
    case VK_ESCAPE: return static_cast<e00::input_value_t>(WinKey::Escape);
    case VK_RETURN: return static_cast<e00::input_value_t>(WinKey::Enter);
    case VK_SPACE: return static_cast<e00::input_value_t>(WinKey::Space);
    case VK_LEFT: return static_cast<e00::input_value_t>(WinKey::Left);
    case VK_RIGHT: return static_cast<e00::input_value_t>(WinKey::Right);
    case VK_UP: return static_cast<e00::input_value_t>(WinKey::Up);
    case VK_DOWN: return static_cast<e00::input_value_t>(WinKey::Down);
    case VK_TAB: return static_cast<e00::input_value_t>(WinKey::Tab);
    case VK_BACK: return static_cast<e00::input_value_t>(WinKey::Backspace);
    case VK_SHIFT: return static_cast<e00::input_value_t>(WinKey::Shift);
    case VK_CONTROL: return static_cast<e00::input_value_t>(WinKey::Control);
    case VK_MENU: return static_cast<e00::input_value_t>(WinKey::Alt);
    default:
      break;
  }

  if (vk >= 'A' && vk <= 'Z') {
    return static_cast<e00::input_value_t>(20 + (vk - 'A'));
  }
  if (vk >= '0' && vk <= '9') {
    return static_cast<e00::input_value_t>(50 + (vk - '0'));
  }
  if (vk >= VK_F1 && vk <= VK_F12) {
    return static_cast<e00::input_value_t>(100 + (vk - VK_F1));
  }
  return static_cast<e00::input_value_t>(vk);
}

}// namespace

void InitKeyboard() {}
void QuitKeyboard() {}

void HandleKeyboardMessage(e00::Engine &engine, UINT msg, WPARAM wParam, LPARAM /*lParam*/) {
  const bool down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
  const bool up = (msg == WM_KEYUP || msg == WM_SYSKEYUP);
  if (!down && !up) {
    return;
  }

  const auto id = MapVirtualKey(wParam);
  e00::InputEvent event;
  event.assign(down ? e00::InputEvent::Type::KeyDown : e00::InputEvent::Type::KeyUp,
               id, WinKeyboardSystem);

  if (down && event.is(WinKeyboardSystem, WinKey::Escape)) {
    engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
  }

  if (down) {
    engine.ProcessInputEvent(event);
  }
}

}// namespace win31
