#include "KeyboardEventHandler.hpp"

#include <pc.h>
#include <go32.h>
#include <dos.h>

constexpr auto KEYBOARD_BUFFER_SIZE = 512;
constexpr auto KEYBOARD_ISR = 9;

namespace {
class KeyboardInputSystem : public e00::InputSystem {
public:
  std::string name() const override {
    return "keybaord";
  }
  std::string name(uint16_t value) const override {
    switch (static_cast<Key>(value)) {
      case KEY_UNKNOWN: break;
      case KEY_A: return "A";
      case KEY_B: return "B";
      case KEY_C: return "C";
      case KEY_D: return "D";
      case KEY_E: return "E";
      case KEY_F: return "F";
      case KEY_G: return "G";
      case KEY_H: return "H";
      case KEY_I: return "I";
      case KEY_J: return "J";
      case KEY_K: return "K";
      case KEY_L: return "L";
      case KEY_M: return "M";
      case KEY_N: return "N";
      case KEY_O: return "O";
      case KEY_P: return "P";
      case KEY_Q: return "Q";
      case KEY_R: return "R";
      case KEY_S: return "S";
      case KEY_T: return "T";
      case KEY_U: return "U";
      case KEY_V: return "V";
      case KEY_W: return "W";
      case KEY_X: return "X";
      case KEY_Y: return "Y";
      case KEY_Z: return "Z";
      case KEY_0: return "0";
      case KEY_1: return "1";
      case KEY_2: return "2";
      case KEY_3: return "3";
      case KEY_4: return "4";
      case KEY_5: return "5";
      case KEY_6: return "6";
      case KEY_7: return "7";
      case KEY_8: return "8";
      case KEY_9: return "9";
      case KEY_0_PAD: return "0_PAD";
      case KEY_1_PAD: return "1_PAD";
      case KEY_2_PAD: return "2_PAD";
      case KEY_3_PAD: return "3_PAD";
      case KEY_4_PAD: return "4_PAD";
      case KEY_5_PAD: return "5_PAD";
      case KEY_6_PAD: return "6_PAD";
      case KEY_7_PAD: return "7_PAD";
      case KEY_8_PAD: return "8_PAD";
      case KEY_9_PAD: return "9_PAD";
      case KEY_F1: return "F1";
      case KEY_F2: return "F2";
      case KEY_F3: return "F3";
      case KEY_F4: return "F4";
      case KEY_F5: return "F5";
      case KEY_F6: return "F6";
      case KEY_F7: return "F7";
      case KEY_F8: return "F8";
      case KEY_F9: return "F9";
      case KEY_F10: return "F10";
      case KEY_F11: return "F11";
      case KEY_F12: return "F12";
      case KEY_ESC: return "ESC";
      case KEY_TILDE: return "TILDE";
      case KEY_MINUS: return "MINUS";
      case KEY_EQUALS: return "EQUALS";
      case KEY_BACKSPACE: return "BACKSPACE";
      case KEY_TAB: return "TAB";
      case KEY_OPENBRACE: return "OPENBRACE";
      case KEY_CLOSEBRACE: return "CLOSEBRACE";
      case KEY_ENTER: return "ENTER";
      case KEY_COLON: return "COLON";
      case KEY_QUOTE: return "QUOTE";
      case KEY_BACKSLASH: return "BACKSLASH";
      case KEY_COMMA: return "COMMA";
      case KEY_STOP: return "STOP";
      case KEY_SLASH: return "SLASH";
      case KEY_SPACE: return "SPACE";
      case KEY_INSERT: return "INSERT";
      case KEY_DEL: return "DEL";
      case KEY_HOME: return "HOME";
      case KEY_END: return "END";
      case KEY_PGUP: return "PGUP";
      case KEY_PGDN: return "PGDN";
      case KEY_LEFT: return "LEFT";
      case KEY_RIGHT: return "RIGHT";
      case KEY_UP: return "UP";
      case KEY_DOWN: return "DOWN";
      case KEY_SLASH_PAD: return "SLASH_PAD";
      case KEY_ASTERISK: return "ASTERISK";
      case KEY_MINUS_PAD: return "MINUS_PAD";
      case KEY_PLUS_PAD: return "PLUS_PAD";
      case KEY_DEL_PAD: return "DEL_PAD";
      case KEY_ENTER_PAD: return "ENTER_PAD";
      case KEY_PRTSCR: return "PRTSCR";
      case KEY_PAUSE: return "PAUSE";
      case KEY_ABNT_C1: return "ABNT_C1";
      case KEY_YEN: return "YEN";
      case KEY_KANA: return "KANA";
      case KEY_CONVERT: return "CONVERT";
      case KEY_NOCONVERT: return "NOCONVERT";
      case KEY_AT: return "AT";
      case KEY_CIRCUMFLEX: return "CIRCUMFLEX";
      case KEY_COLON2: return "COLON2";
      case KEY_KANJI: return "KANJI";
      case KEY_SYSRQ: return "SYSRQ";
      case KEY_LSHIFT: return "LSHIFT";
      case KEY_RSHIFT: return "RSHIFT";
      case KEY_LCONTROL: return "LCONTROL";
      case KEY_RCONTROL: return "RCONTROL";
      case KEY_ALT: return "ALT";
      case KEY_ALTGR: return "ALTGR";
      case KEY_LWIN: return "LWIN";
      case KEY_RWIN: return "RWIN";
      case KEY_MENU: return "MENU";
      case KEY_SCRLOCK: return "SCRLOCK";
      case KEY_NUMLOCK: return "NUMLOCK";
      case KEY_CAPSLOCK: return "CAPSLOCK";
    }
    return "unknown";
  }
};

const KeyboardInputSystem keyboard_input_system;
}// namespace

const e00::InputSystem &KeyboardInputSystem() noexcept {
  return keyboard_input_system;
}

struct KeyboardHandlerCriticalData {
  int intr_next_index;
  uint8_t keys[KEYBOARD_BUFFER_SIZE];
};

namespace {
volatile struct KeyboardHandlerCriticalData critical_data;

void keyboard_handler() {
  auto ints = disable();
  uint8_t code = inportb(0x60);

  int i = critical_data.intr_next_index;
  critical_data.keys[i] = code;

  if (i >= KEYBOARD_BUFFER_SIZE - 1) {
    critical_data.intr_next_index = 0;
  } else {
    critical_data.intr_next_index = i + 1;
  }

  outportb(0x20, 0x20);
  if (ints) enable();
}
}// namespace

KeyboardEventHandler::KeyboardEventHandler() noexcept
  : next_key(0) {
  _go32_dpmi_get_protected_mode_interrupt_vector(KEYBOARD_ISR, &keyboardOldISR);

  critical_data.intr_next_index = 0;

  _go32_dpmi_lock_data((void *)&critical_data, sizeof(critical_data));
  _go32_dpmi_lock_code((void *)&keyboard_handler, 4096);
  keyboardNewISR.pm_offset = (unsigned long)&keyboard_handler;
  keyboardNewISR.pm_selector = (unsigned short)_my_cs();

  _go32_dpmi_allocate_iret_wrapper(&keyboardNewISR);
  _go32_dpmi_set_protected_mode_interrupt_vector(KEYBOARD_ISR, &keyboardNewISR);
}

KeyboardEventHandler::~KeyboardEventHandler() noexcept {
  _go32_dpmi_set_protected_mode_interrupt_vector(KEYBOARD_ISR, &keyboardOldISR);
  _go32_dpmi_free_iret_wrapper(&keyboardNewISR);
}

e00::InputEvent KeyboardEventHandler::event() noexcept {
  if (next_key >= KEYBOARD_BUFFER_SIZE) {
    next_key = 0;
  }

  if (critical_data.intr_next_index > next_key) {
    const auto scancode = critical_data.keys[next_key++];
    if (scancode == 0) {
      return {};
    }

    // the pause sequence needs to be first
    const auto pause_result = _pause_handler.handle(scancode);
    if (!pause_result.should_continue_processing) {
      if (pause_result.has_event_data) {
        return pause_result.event;
      }

      return {};
    }

    // other extended keys will start with 0xE0
    const auto extended_result = _extended_handler.handle(scancode);
    if (!extended_result.should_continue_processing) {
      if (extended_result.has_event_data) {
        return extended_result.event;
      }

      return {};
    }

    // Normal keys!
    return _normal_handler.handle(scancode).event;
  }

  return {};
}
