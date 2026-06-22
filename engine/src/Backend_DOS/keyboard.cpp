
#include "keyboard.hpp"
#include "interrupt.hpp"
#include "memory.hpp"

#include <array>
#include <stdint.h>

namespace {

constexpr uint16_t KBD_DATA_PORT = 0x60;
// Scancode byte structure
constexpr uint8_t SCANCODE_MASK = 0x7F;   // bits 0-6: scancode index
constexpr uint8_t SCANCODE_RELEASE = 0x80;// bit 7: key released (break code)

// Multi-byte scancode prefixes
constexpr uint8_t SCANCODE_PREFIX_EXTENDED = 0xE0;// extended key prefix
constexpr uint8_t SCANCODE_PREFIX_PAUSE = 0xE1;   // pause key sequence prefix

DOS_InterruptHook keyboard_interrupt_hook;
std::array<uint8_t, 256> keyevents_ringbuffer;
volatile int keyevents_head = 0;
volatile int keyevents_tail = 0;

void KeyboardIRQHandler() {
  keyevents_ringbuffer[keyevents_head] = inportb(KBD_DATA_PORT);
  keyevents_head = (keyevents_head + 1) & (keyevents_ringbuffer.size() - 1);
  // Do not chain interrupts!
  DOS_EndOfInterrupt(1);
}
void KeyboardIRQHandler_End() {}// end-of-ISR label for memory locking

enum class DosNormalInput : e00::input_value_t {
  // clang-format off
  KEY_UNKNOWN,
  
  KEY_ESCAPE,
  KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
  KEY_1,  KEY_2,  KEY_3,  KEY_4,  KEY_5,  KEY_6,  KEY_7,  KEY_8,  KEY_9,  KEY_0,
  KEY_Q,  KEY_W,  KEY_E,  KEY_R,  KEY_T,  KEY_Y,  KEY_U,  KEY_I,  KEY_O,  KEY_P,
  KEY_A,  KEY_S,  KEY_D,  KEY_F,  KEY_G,  KEY_H,  KEY_J,  KEY_K,  KEY_L,
  KEY_Z,  KEY_X,  KEY_C,  KEY_V,  KEY_B,  KEY_N,  KEY_M,
  KEY_MINUS, KEY_EQUALS, KEY_SPACE, KEY_ENTER, KEY_BACKSPACE, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET,
  KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT, KEY_LEFT_CTRL, KEY_LEFT_ALT,
  KEY_TAB, KEY_CAPS_LOCK, KEY_BACKSLASH, KEY_SLASH, KEY_PERIOD, KEY_COMMA,
  KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE,
  
  KEY_KP_MULTIPLY, KEY_NUMLOCKCLEAR, KEY_SCROLLLOCK,
  KEY_KP_7,  KEY_KP_8,  KEY_KP_9,  KEY_KP_MINUS,  KEY_KP_4,  KEY_KP_5,
  KEY_KP_6,  KEY_KP_PLUS,  KEY_KP_1,  KEY_KP_2,  KEY_KP_3,  KEY_KP_0,
  KEY_KP_PERIOD,
  // clang-format on
};
class DosInputSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override { return "DOS Keyboard"; }
  [[nodiscard]] std::string name(e00::input_value_t value) const override {
    // clang-format off
    switch (static_cast<DosNormalInput>(value)) {
      case DosNormalInput::KEY_UNKNOWN: return "Unknown";
      case DosNormalInput::KEY_ESCAPE: return "Escape";
      case DosNormalInput::KEY_F1: return "F1";
      case DosNormalInput::KEY_F2: return "F2";
      case DosNormalInput::KEY_F3: return "F3";
      case DosNormalInput::KEY_F4: return "F4";
      case DosNormalInput::KEY_F5: return "F5";
      case DosNormalInput::KEY_F6: return "F6";
      case DosNormalInput::KEY_F7: return "F7";
      case DosNormalInput::KEY_F8: return "F8";
      case DosNormalInput::KEY_F9: return "F9";
      case DosNormalInput::KEY_F10: return "F10";
      case DosNormalInput::KEY_F11: return "F11";
      case DosNormalInput::KEY_F12: return "F12";
      case DosNormalInput::KEY_1: return "1";
      case DosNormalInput::KEY_2: return "2";
      case DosNormalInput::KEY_3: return "3";
      case DosNormalInput::KEY_4: return "4";
      case DosNormalInput::KEY_5: return "5";
      case DosNormalInput::KEY_6: return "6";
      case DosNormalInput::KEY_7: return "7";
      case DosNormalInput::KEY_8: return "8";
      case DosNormalInput::KEY_9: return "9";
      case DosNormalInput::KEY_0: return "0";
      case DosNormalInput::KEY_Q: return "Q";
      case DosNormalInput::KEY_W: return "W";
      case DosNormalInput::KEY_E: return "E";
      case DosNormalInput::KEY_R: return "R";
      case DosNormalInput::KEY_T: return "T";
      case DosNormalInput::KEY_Y: return "Y";
      case DosNormalInput::KEY_U: return "U";
      case DosNormalInput::KEY_I: return "I";
      case DosNormalInput::KEY_O: return "O";
      case DosNormalInput::KEY_P: return "P";
      case DosNormalInput::KEY_A: return "A";
      case DosNormalInput::KEY_S: return "S";
      case DosNormalInput::KEY_D: return "D";
      case DosNormalInput::KEY_F: return "F";
      case DosNormalInput::KEY_G: return "G";
      case DosNormalInput::KEY_H: return "H";
      case DosNormalInput::KEY_J: return "J";
      case DosNormalInput::KEY_K: return "K";
      case DosNormalInput::KEY_L: return "L";
      case DosNormalInput::KEY_Z: return "Z";
      case DosNormalInput::KEY_X: return "X";
      case DosNormalInput::KEY_C: return "C";
      case DosNormalInput::KEY_V: return "V";
      case DosNormalInput::KEY_B: return "B";
      case DosNormalInput::KEY_N: return "N";
      case DosNormalInput::KEY_M: return "M";
      case DosNormalInput::KEY_MINUS: return "-";
      case DosNormalInput::KEY_EQUALS: return "=";
      case DosNormalInput::KEY_SPACE: return "Space";
      case DosNormalInput::KEY_ENTER: return "Enter";
      case DosNormalInput::KEY_BACKSPACE: return "Backspace";
      case DosNormalInput::KEY_LEFT_BRACKET: return "[";
      case DosNormalInput::KEY_RIGHT_BRACKET: return "]";
      case DosNormalInput::KEY_LEFT_SHIFT: return "Left Shift";
      case DosNormalInput::KEY_RIGHT_SHIFT: return "Right Shift";
      case DosNormalInput::KEY_LEFT_CTRL: return "Left Ctrl";
      case DosNormalInput::KEY_LEFT_ALT: return "Left Alt";
      case DosNormalInput::KEY_TAB: return "Tab";
      case DosNormalInput::KEY_CAPS_LOCK: return "Caps Lock";
      case DosNormalInput::KEY_BACKSLASH: return "\\";
      case DosNormalInput::KEY_SLASH: return "/";
      case DosNormalInput::KEY_PERIOD: return ".";
      case DosNormalInput::KEY_COMMA: return ",";
      case DosNormalInput::KEY_SEMICOLON: return ";";
      case DosNormalInput::KEY_APOSTROPHE: return "'";
      case DosNormalInput::KEY_GRAVE: return "`";
      case DosNormalInput::KEY_KP_MULTIPLY: return "*";
      case DosNormalInput::KEY_NUMLOCKCLEAR: return "Num Lock";
      case DosNormalInput::KEY_SCROLLLOCK: return "Scroll Lock";
      case DosNormalInput::KEY_KP_7: return "Keypad 7";
      case DosNormalInput::KEY_KP_8: return "Keypad 8";
      case DosNormalInput::KEY_KP_9: return "Keypad 9";
      case DosNormalInput::KEY_KP_MINUS: return "Keypad -";
      case DosNormalInput::KEY_KP_4: return "Keypad 4";
      case DosNormalInput::KEY_KP_5: return "Keypad 5";
      case DosNormalInput::KEY_KP_6: return "Keypad 6";
      case DosNormalInput::KEY_KP_PLUS: return "Keypad +";
      case DosNormalInput::KEY_KP_1: return "Keypad 1";
      case DosNormalInput::KEY_KP_2: return "Keypad 2";
      case DosNormalInput::KEY_KP_3: return "Keypad 3";
      case DosNormalInput::KEY_KP_0: return "Keypad 0";
      case DosNormalInput::KEY_KP_PERIOD: return "Keypad .";
    }
    // clang-format on
    return {};
  }
};
const e00::InputSystem &DosKeyboardInputSystem = DosInputSystem{};

enum class DosSpecialInput : e00::input_value_t {
  // clang-format off
  KEY_UNKNOWN,
  KEY_KP_DIVIDE,  KEY_KP_ENTER,  KEY_RIGHT_ALT,  KEY_RIGHT_CTRL,
  KEY_INSERT,  KEY_HOME,  KEY_PAGE_UP,  KEY_DELETE,  KEY_END,
  KEY_PAGE_DOWN,  KEY_ARROW_UP,  KEY_ARROW_DOWN,  KEY_ARROW_LEFT,
  KEY_ARROW_RIGHT,  KEY_PRINT_SCREEN,  KEY_PAUSE,  KEY_LEFT_GUI,
  KEY_RIGHT_GUI, KEY_APPLICATION
  // clang-format on
};
class DosExtendedSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override { return "DOS Keyboard"; }
  [[nodiscard]] std::string name(e00::input_value_t value) const override {
    // clang-format off
    switch (static_cast<DosSpecialInput>(value)) {
      case DosSpecialInput::KEY_UNKNOWN: return "Unknown";
      case DosSpecialInput::KEY_KP_DIVIDE: return "Keypad /";
      case DosSpecialInput::KEY_KP_ENTER: return "Keypad Enter";
      case DosSpecialInput::KEY_RIGHT_ALT: return "Right Alt";
      case DosSpecialInput::KEY_RIGHT_CTRL: return "Right Ctrl";
      case DosSpecialInput::KEY_INSERT: return "Insert";
      case DosSpecialInput::KEY_HOME: return "Home";
      case DosSpecialInput::KEY_PAGE_UP: return "Page Up";
      case DosSpecialInput::KEY_DELETE: return "Delete";
      case DosSpecialInput::KEY_END: return "End";
      case DosSpecialInput::KEY_PAGE_DOWN: return "Page Down";
      case DosSpecialInput::KEY_ARROW_UP: return "Arrow Up";
      case DosSpecialInput::KEY_ARROW_DOWN: return "Arrow Down";
      case DosSpecialInput::KEY_ARROW_LEFT: return "Arrow Left";
      case DosSpecialInput::KEY_ARROW_RIGHT: return "Arrow Right";
      case DosSpecialInput::KEY_PRINT_SCREEN: return "Print Screen";
      case DosSpecialInput::KEY_PAUSE: return "Pause";
      case DosSpecialInput::KEY_LEFT_GUI: return "Left GUI";
      case DosSpecialInput::KEY_RIGHT_GUI: return "Right GUI";
      case DosSpecialInput::KEY_APPLICATION: return "Application";
    }
    // clang-format on
  }
};

const e00::InputSystem &DosExtendedKeyboardInputSystem = DosExtendedSystem{};

std::array DosMapping{
    // index is the scancode from the IRQ1 handler bitwise-ANDed against 0x7F.
    /* 0x00 */ DosNormalInput::KEY_UNKNOWN,
    /* 0x01 */ DosNormalInput::KEY_ESCAPE,
    /* 0x02 */ DosNormalInput::KEY_1,
    /* 0x03 */ DosNormalInput::KEY_2,
    /* 0x04 */ DosNormalInput::KEY_3,
    /* 0x05 */ DosNormalInput::KEY_4,
    /* 0x06 */ DosNormalInput::KEY_5,
    /* 0x07 */ DosNormalInput::KEY_6,
    /* 0x08 */ DosNormalInput::KEY_7,
    /* 0x09 */ DosNormalInput::KEY_8,
    /* 0x0A */ DosNormalInput::KEY_9,
    /* 0x0B */ DosNormalInput::KEY_0,
    /* 0x0C */ DosNormalInput::KEY_MINUS,
    /* 0x0D */ DosNormalInput::KEY_EQUALS,
    /* 0x0E */ DosNormalInput::KEY_BACKSPACE,
    /* 0x0F */ DosNormalInput::KEY_TAB,

    /* 0x10 */ DosNormalInput::KEY_Q,
    /* 0x11 */ DosNormalInput::KEY_W,
    /* 0x12 */ DosNormalInput::KEY_E,
    /* 0x13 */ DosNormalInput::KEY_R,
    /* 0x14 */ DosNormalInput::KEY_T,
    /* 0x15 */ DosNormalInput::KEY_Y,
    /* 0x16 */ DosNormalInput::KEY_U,
    /* 0x17 */ DosNormalInput::KEY_I,
    /* 0x18 */ DosNormalInput::KEY_O,
    /* 0x19 */ DosNormalInput::KEY_P,
    /* 0x1A */ DosNormalInput::KEY_LEFT_BRACKET,
    /* 0x1B */ DosNormalInput::KEY_RIGHT_BRACKET,
    /* 0x1C */ DosNormalInput::KEY_ENTER,
    /* 0x1D */ DosNormalInput::KEY_LEFT_CTRL,
    /* 0x1E */ DosNormalInput::KEY_A,
    /* 0x1F */ DosNormalInput::KEY_S,

    /* 0x20 */ DosNormalInput::KEY_D,
    /* 0x21 */ DosNormalInput::KEY_F,
    /* 0x22 */ DosNormalInput::KEY_G,
    /* 0x23 */ DosNormalInput::KEY_H,
    /* 0x24 */ DosNormalInput::KEY_J,
    /* 0x25 */ DosNormalInput::KEY_K,
    /* 0x26 */ DosNormalInput::KEY_L,
    /* 0x27 */ DosNormalInput::KEY_SEMICOLON,
    /* 0x28 */ DosNormalInput::KEY_APOSTROPHE,
    /* 0x29 */ DosNormalInput::KEY_GRAVE,
    /* 0x2A */ DosNormalInput::KEY_LEFT_SHIFT,
    /* 0x2B */ DosNormalInput::KEY_BACKSLASH,
    /* 0x2C */ DosNormalInput::KEY_Z,
    /* 0x2D */ DosNormalInput::KEY_X,
    /* 0x2E */ DosNormalInput::KEY_C,
    /* 0x2F */ DosNormalInput::KEY_V,

    /* 0x30 */ DosNormalInput::KEY_B,
    /* 0x31 */ DosNormalInput::KEY_N,
    /* 0x32 */ DosNormalInput::KEY_M,
    /* 0x33 */ DosNormalInput::KEY_COMMA,
    /* 0x34 */ DosNormalInput::KEY_PERIOD,
    /* 0x35 */ DosNormalInput::KEY_SLASH,
    /* 0x36 */ DosNormalInput::KEY_RIGHT_SHIFT,
    /* 0x37 */ DosNormalInput::KEY_KP_MULTIPLY,
    /* 0x38 */ DosNormalInput::KEY_LEFT_ALT,
    /* 0x39 */ DosNormalInput::KEY_SPACE,
    /* 0x3A */ DosNormalInput::KEY_CAPS_LOCK,
    /* 0x3B */ DosNormalInput::KEY_F1,
    /* 0x3C */ DosNormalInput::KEY_F2,
    /* 0x3D */ DosNormalInput::KEY_F3,
    /* 0x3E */ DosNormalInput::KEY_F4,
    /* 0x3F */ DosNormalInput::KEY_F5,

    /* 0x040 */ DosNormalInput::KEY_F6,
    /* 0x041 */ DosNormalInput::KEY_F7,
    /* 0x042 */ DosNormalInput::KEY_F8,
    /* 0x043 */ DosNormalInput::KEY_F9,
    /* 0x044 */ DosNormalInput::KEY_F10,
    /* 0x045 */ DosNormalInput::KEY_NUMLOCKCLEAR,
    /* 0x046 */ DosNormalInput::KEY_SCROLLLOCK,
    /* 0x047 */ DosNormalInput::KEY_KP_7,
    /* 0x048 */ DosNormalInput::KEY_KP_8,
    /* 0x049 */ DosNormalInput::KEY_KP_9,
    /* 0x04A */ DosNormalInput::KEY_KP_MINUS,
    /* 0x04B */ DosNormalInput::KEY_KP_4,
    /* 0x04C */ DosNormalInput::KEY_KP_5,
    /* 0x04D */ DosNormalInput::KEY_KP_6,
    /* 0x04E */ DosNormalInput::KEY_KP_PLUS,
    /* 0x04F */ DosNormalInput::KEY_KP_1,

    /* 0x050 */ DosNormalInput::KEY_KP_2,
    /* 0x051 */ DosNormalInput::KEY_KP_3,
    /* 0x052 */ DosNormalInput::KEY_KP_0,
    /* 0x053 */ DosNormalInput::KEY_KP_PERIOD,
    /* 0x054 */ DosNormalInput::KEY_UNKNOWN,
    /* 0x055 */ DosNormalInput::KEY_UNKNOWN,
    /* 0x056 */ DosNormalInput::KEY_UNKNOWN,
    /* 0x057 */ DosNormalInput::KEY_F11,
    /* 0x058 */ DosNormalInput::KEY_F12};

std::array DosExtendedMapping{
    /* 0x00 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x01 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x02 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x03 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x04 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x05 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x06 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x07 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x08 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x09 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0A */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0B */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0C */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0D */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0E */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x0F */ DosSpecialInput::KEY_UNKNOWN,

    /* 0x10 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x11 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x12 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x13 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x14 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x15 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x16 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x17 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x18 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x19 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x1A */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x1B */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x1C */ DosSpecialInput::KEY_KP_ENTER,
    /* 0x1D */ DosSpecialInput::KEY_RIGHT_CTRL,
    /* 0x1E */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x1F */ DosSpecialInput::KEY_UNKNOWN,

    /* 0x20 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x21 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x22 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x23 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x24 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x25 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x26 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x27 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x28 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x29 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x2A */ DosSpecialInput::KEY_UNKNOWN,// fake left shift, ignore
    /* 0x2B */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x2C */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x2D */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x2E */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x2F */ DosSpecialInput::KEY_UNKNOWN,

    /* 0x30 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x31 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x32 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x33 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x34 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x35 */ DosSpecialInput::KEY_KP_DIVIDE,
    /* 0x36 */ DosSpecialInput::KEY_UNKNOWN,// fake right shift, ignore
    /* 0x37 */ DosSpecialInput::KEY_PRINT_SCREEN,
    /* 0x38 */ DosSpecialInput::KEY_RIGHT_ALT,
    /* 0x39 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3A */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3B */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3C */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3D */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3E */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x3F */ DosSpecialInput::KEY_UNKNOWN,

    /* 0x40 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x41 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x42 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x43 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x44 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x45 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x46 */ DosSpecialInput::KEY_PAUSE,// Ctrl+Break sends E0 46 E0 C6
    /* 0x47 */ DosSpecialInput::KEY_HOME,
    /* 0x48 */ DosSpecialInput::KEY_ARROW_UP,
    /* 0x49 */ DosSpecialInput::KEY_PAGE_UP,
    /* 0x4A */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x4B */ DosSpecialInput::KEY_ARROW_LEFT,
    /* 0x4C */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x4D */ DosSpecialInput::KEY_ARROW_RIGHT,
    /* 0x4E */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x4F */ DosSpecialInput::KEY_END,

    /* 0x50 */ DosSpecialInput::KEY_ARROW_DOWN,
    /* 0x51 */ DosSpecialInput::KEY_PAGE_DOWN,
    /* 0x52 */ DosSpecialInput::KEY_INSERT,
    /* 0x53 */ DosSpecialInput::KEY_DELETE,
    /* 0x54 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x55 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x56 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x57 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x58 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x59 */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x5A */ DosSpecialInput::KEY_UNKNOWN,
    /* 0x5B */ DosSpecialInput::KEY_LEFT_GUI,
    /* 0x5C */ DosSpecialInput::KEY_RIGHT_GUI,
    /* 0x5D */ DosSpecialInput::KEY_APPLICATION};
}// namespace

void InitKeyboard() {
  // Lock ISR code and data to prevent page faults during interrupts
  DOS_LockCode(KeyboardIRQHandler, KeyboardIRQHandler_End);
  DOS_LockVariable(keyevents_ringbuffer);
  DOS_LockVariable(keyevents_head);
  DOS_LockVariable(keyevents_tail);

  keyevents_head = 0;
  keyevents_tail = 0;

  DOS_DisableInterrupts();
  DOS_HookInterrupt(1, KeyboardIRQHandler, &keyboard_interrupt_hook);
  DOS_EnableInterrupts();
}

void QuitKeyboard() {
  DOS_DisableInterrupts();
  DOS_UnhookInterrupt(&keyboard_interrupt_hook, false);
  DOS_EnableInterrupts();

  {
    // Acknowledge/Reset command to the controller keyboard port
    outportb(0x61, inportb(0x61) | 0x80);// Pull clock line low (disable keyboard)
    outportb(0x61, inportb(0x61) & 0x7F);// Release clock line (enable keyboard)
  }

  // Drain the BIOS keyboard buffer so held keys (like ESC) don't
  // bleed through to the DOS command line after we exit.
  {
    __dpmi_regs regs;
    for (;;) {
      regs.h.ah = 0x01;// BIOS: check for keystroke
      __dpmi_int(0x16, &regs);
      if (regs.x.flags & 0x40) {// ZF set = buffer empty
        break;
      }
      regs.h.ah = 0x00;// BIOS: read keystroke (removes it)
      __dpmi_int(0x16, &regs);
    }
  }
}

bool SendKeyboardEvent(e00::Engine &engine) {
  static bool is_extended = false;
  static int pause_sequence_remaining = 0;

  int current_head = keyevents_head;

  while (current_head != keyevents_tail) {
    const auto event = keyevents_ringbuffer[keyevents_tail];
    keyevents_tail = (keyevents_tail + 1) & (keyevents_ringbuffer.size() - 1);

    // Handle remaining bytes of E1 Pause key sequence (E1 1D 45 E1 9D C5).
    if (pause_sequence_remaining > 0) {
      pause_sequence_remaining--;
      continue;
    }

    // Pause key sends a multi-byte sequence: E1 1D 45 E1 9D C5. Emit PAUSE press+release and consume the rest.
    if (event == SCANCODE_PREFIX_PAUSE) {
      pause_sequence_remaining = 5;// skip the next 5 bytes
      // TODO: Send input event PAUSE
      continue;
    }

    if (event == SCANCODE_PREFIX_EXTENDED) {
      is_extended = true;
      continue;
    }

    const auto scancode = event & SCANCODE_MASK;
    const auto pressed = ((event & SCANCODE_RELEASE) == 0);
    const auto current_event_type = pressed ? e00::InputEvent::Type::KeyDown : e00::InputEvent::Type::KeyUp;

    e00::InputEvent mapping;

    if (!is_extended) {
      if (scancode < DosMapping.size()) {
        mapping.assign(
            current_event_type,
            static_cast<e00::input_value_t>(DosMapping[scancode]),
            DosKeyboardInputSystem);
      }
    } else {
      if (scancode < DosExtendedMapping.size()) {
        mapping.assign(
            current_event_type,
            static_cast<e00::input_value_t>(DosExtendedMapping[scancode]),
            DosExtendedKeyboardInputSystem);
      }

      is_extended = false;
    }

    if (mapping && mapping.type() == e00::InputEvent::Type::KeyDown) {
      if (mapping.is(DosKeyboardInputSystem, DosNormalInput::KEY_ESCAPE)) {
        engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
      }

      engine.ProcessInputEvent(mapping);
    }
  }
  return true;
}
