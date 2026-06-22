#include "Apple_KeyboardSystem.h"

#import <AppKit/AppKit.h>

namespace {
constexpr platform::Apple_KeyboardSystem keyboard_system{};

constexpr uint32_t kVK_ANSI_A = 0x00;
constexpr uint32_t kVK_ANSI_S = 0x01;
constexpr uint32_t kVK_ANSI_D = 0x02;
constexpr uint32_t kVK_ANSI_F = 0x03;
constexpr uint32_t kVK_ANSI_H = 0x04;
constexpr uint32_t kVK_ANSI_G = 0x05;
constexpr uint32_t kVK_ANSI_Z = 0x06;
constexpr uint32_t kVK_ANSI_X = 0x07;
constexpr uint32_t kVK_ANSI_C = 0x08;
constexpr uint32_t kVK_ANSI_V = 0x09;
constexpr uint32_t kVK_ANSI_B = 0x0B;
constexpr uint32_t kVK_ANSI_Q = 0x0C;
constexpr uint32_t kVK_ANSI_W = 0x0D;
constexpr uint32_t kVK_ANSI_E = 0x0E;
constexpr uint32_t kVK_ANSI_R = 0x0F;
constexpr uint32_t kVK_ANSI_Y = 0x10;
constexpr uint32_t kVK_ANSI_T = 0x11;
constexpr uint32_t kVK_ANSI_1 = 0x12;
constexpr uint32_t kVK_ANSI_2 = 0x13;
constexpr uint32_t kVK_ANSI_3 = 0x14;
constexpr uint32_t kVK_ANSI_4 = 0x15;
constexpr uint32_t kVK_ANSI_6 = 0x16;
constexpr uint32_t kVK_ANSI_5 = 0x17;
constexpr uint32_t kVK_ANSI_Equal = 0x18;
constexpr uint32_t kVK_ANSI_9 = 0x19;
constexpr uint32_t kVK_ANSI_7 = 0x1A;
constexpr uint32_t kVK_ANSI_Minus = 0x1B;
constexpr uint32_t kVK_ANSI_8 = 0x1C;
constexpr uint32_t kVK_ANSI_0 = 0x1D;
constexpr uint32_t kVK_ANSI_RightBracket = 0x1E;
constexpr uint32_t kVK_ANSI_O = 0x1F;
constexpr uint32_t kVK_ANSI_U = 0x20;
constexpr uint32_t kVK_ANSI_LeftBracket = 0x21;
constexpr uint32_t kVK_ANSI_I = 0x22;
constexpr uint32_t kVK_ANSI_P = 0x23;
constexpr uint32_t kVK_ANSI_L = 0x25;
constexpr uint32_t kVK_ANSI_J = 0x26;
constexpr uint32_t kVK_ANSI_Quote = 0x27;
constexpr uint32_t kVK_ANSI_K = 0x28;
constexpr uint32_t kVK_ANSI_Semicolon = 0x29;
constexpr uint32_t kVK_ANSI_Backslash = 0x2A;
constexpr uint32_t kVK_ANSI_Comma = 0x2B;
constexpr uint32_t kVK_ANSI_Slash = 0x2C;
constexpr uint32_t kVK_ANSI_N = 0x2D;
constexpr uint32_t kVK_ANSI_M = 0x2E;
constexpr uint32_t kVK_ANSI_Period = 0x2F;
constexpr uint32_t kVK_ANSI_Grave = 0x32;

constexpr uint32_t kVK_Return = 0x24;
constexpr uint32_t kVK_Tab = 0x30;
constexpr uint32_t kVK_Space = 0x31;
constexpr uint32_t kVK_Delete = 0x33;
constexpr uint32_t kVK_Escape = 0x35;
constexpr uint32_t kVK_Command = 0x37;
constexpr uint32_t kVK_Shift = 0x38;
constexpr uint32_t kVK_CapsLock = 0x39;
constexpr uint32_t kVK_Option = 0x3A;
constexpr uint32_t kVK_Control = 0x3B;
constexpr uint32_t kVK_RightShift = 0x3C;
constexpr uint32_t kVK_RightOption = 0x3D;
constexpr uint32_t kVK_RightControl = 0x3E;
constexpr uint32_t kVK_Function = 0x3F;
constexpr uint32_t kVK_F17 = 0x40;
constexpr uint32_t kVK_VolumeUp = 0x48;
constexpr uint32_t kVK_VolumeDown = 0x49;
constexpr uint32_t kVK_Mute = 0x4A;
constexpr uint32_t kVK_F18 = 0x4F;
constexpr uint32_t kVK_F19 = 0x50;
constexpr uint32_t kVK_F20 = 0x5A;
constexpr uint32_t kVK_F5 = 0x60;
constexpr uint32_t kVK_F6 = 0x61;
constexpr uint32_t kVK_F7 = 0x62;
constexpr uint32_t kVK_F3 = 0x63;
constexpr uint32_t kVK_F8 = 0x64;
constexpr uint32_t kVK_F9 = 0x65;
constexpr uint32_t kVK_F11 = 0x67;
constexpr uint32_t kVK_F13 = 0x69;
constexpr uint32_t kVK_F16 = 0x6A;
constexpr uint32_t kVK_F14 = 0x6B;
constexpr uint32_t kVK_F10 = 0x6D;
constexpr uint32_t kVK_F12 = 0x6F;
constexpr uint32_t kVK_F15 = 0x71;
constexpr uint32_t kVK_Help = 0x72;
constexpr uint32_t kVK_Home = 0x73;
constexpr uint32_t kVK_PageUp = 0x74;
constexpr uint32_t kVK_ForwardDelete = 0x75;
constexpr uint32_t kVK_F4 = 0x76;
constexpr uint32_t kVK_End = 0x77;
constexpr uint32_t kVK_F2 = 0x78;
constexpr uint32_t kVK_PageDown = 0x79;
constexpr uint32_t kVK_F1 = 0x7A;
constexpr uint32_t kVK_LeftArrow = 0x7B;
constexpr uint32_t kVK_RightArrow = 0x7C;
constexpr uint32_t kVK_DownArrow = 0x7D;
constexpr uint32_t kVK_UpArrow = 0x7E;
}// namespace

namespace platform {

std::string Apple_KeyboardSystem::name(e00::input_value_t value) const {
  switch (value) {
    case kVK_ANSI_A: return "A";
    case kVK_ANSI_B: return "B";
    case kVK_ANSI_C: return "C";
    case kVK_ANSI_D: return "D";
    case kVK_ANSI_E: return "E";
    case kVK_ANSI_F: return "F";
    case kVK_ANSI_G: return "G";
    case kVK_ANSI_H: return "H";
    case kVK_ANSI_I: return "I";
    case kVK_ANSI_J: return "J";
    case kVK_ANSI_K: return "K";
    case kVK_ANSI_L: return "L";
    case kVK_ANSI_M: return "M";
    case kVK_ANSI_N: return "N";
    case kVK_ANSI_O: return "O";
    case kVK_ANSI_P: return "P";
    case kVK_ANSI_Q: return "Q";
    case kVK_ANSI_R: return "R";
    case kVK_ANSI_S: return "S";
    case kVK_ANSI_T: return "T";
    case kVK_ANSI_U: return "U";
    case kVK_ANSI_V: return "V";
    case kVK_ANSI_W: return "W";
    case kVK_ANSI_X: return "X";
    case kVK_ANSI_Y: return "Y";
    case kVK_ANSI_Z: return "Z";

    case kVK_ANSI_0: return "0";
    case kVK_ANSI_1: return "1";
    case kVK_ANSI_2: return "2";
    case kVK_ANSI_3: return "3";
    case kVK_ANSI_4: return "4";
    case kVK_ANSI_5: return "5";
    case kVK_ANSI_6: return "6";
    case kVK_ANSI_7: return "7";
    case kVK_ANSI_8: return "8";
    case kVK_ANSI_9: return "9";

    case kVK_Return: return "Return";
    case kVK_Tab: return "Tab";
    case kVK_Space: return "Space";
    case kVK_Delete: return "Backspace";
    case kVK_Escape: return "Escape";
    case kVK_Command: return "Command";
    case kVK_Shift: return "Shift";
    case kVK_CapsLock: return "Caps Lock";
    case kVK_Option: return "Option";
    case kVK_Control: return "Control";
    case kVK_RightShift: return "Right Shift";
    case kVK_RightOption: return "Right Option";
    case kVK_RightControl: return "Right Control";
    case kVK_Function: return "Function";

    case kVK_LeftArrow: return "Left";
    case kVK_RightArrow: return "Right";
    case kVK_DownArrow: return "Down";
    case kVK_UpArrow: return "Up";

    case kVK_Home: return "Home";
    case kVK_End: return "End";
    case kVK_PageUp: return "Page Up";
    case kVK_PageDown: return "Page Down";
    case kVK_ForwardDelete: return "Delete";
    case kVK_Help: return "Help";

    case kVK_F1: return "F1";
    case kVK_F2: return "F2";
    case kVK_F3: return "F3";
    case kVK_F4: return "F4";
    case kVK_F5: return "F5";
    case kVK_F6: return "F6";
    case kVK_F7: return "F7";
    case kVK_F8: return "F8";
    case kVK_F9: return "F9";
    case kVK_F10: return "F10";
    case kVK_F11: return "F11";
    case kVK_F12: return "F12";
    case kVK_F13: return "F13";
    case kVK_F14: return "F14";
    case kVK_F15: return "F15";
    case kVK_F16: return "F16";
    case kVK_F17: return "F17";
    case kVK_F18: return "F18";
    case kVK_F19: return "F19";
    case kVK_F20: return "F20";

    case kVK_VolumeUp: return "Volume Up";
    case kVK_VolumeDown: return "Volume Down";
    case kVK_Mute: return "Mute";

    case kVK_ANSI_Equal: return "=";
    case kVK_ANSI_Minus: return "-";
    case kVK_ANSI_RightBracket: return "]";
    case kVK_ANSI_LeftBracket: return "[";
    case kVK_ANSI_Quote: return "'";
    case kVK_ANSI_Semicolon: return ";";
    case kVK_ANSI_Backslash: return "\\";
    case kVK_ANSI_Comma: return ",";
    case kVK_ANSI_Slash: return "/";
    case kVK_ANSI_Period: return ".";
    case kVK_ANSI_Grave: return "`";

    default:
      return "Key " + std::to_string(value);
  }
}

const Apple_KeyboardSystem &GetKeyboardSystem() {
  return keyboard_system;
}

e00::InputEvent MakeAppleKey(NSEvent *event, e00::InputEvent::Type type) {
  return {keyboard_system, type, static_cast<e00::input_value_t>([event keyCode])};
}

}// namespace platform
