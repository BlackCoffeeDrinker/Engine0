#pragma once

#include <cstdint>
#include "Engine.hpp"

#include "KeyboardEventHandlerResult.hpp"
#include "Keys.hpp"

class KeyboardEventNormalHandler {
  static Key key_from_lower_scancode(uint8_t scancode) {

    switch (scancode) {
      case 0x0E: return Key::KEY_BACKSPACE;
      case 0x3A: return Key::KEY_CAPSLOCK;
      case 0x1C: return Key::KEY_ENTER;
      case 0x01: return Key::KEY_ESC;
      case 0x38: return Key::KEY_ALT;
      case 0x1D: return Key::KEY_LCONTROL;
      case 0x2A: return Key::KEY_LSHIFT;
      case 0x45: return Key::KEY_NUMLOCK;
      case 0x36: return Key::KEY_RSHIFT;
      case 0x46: return Key::KEY_SCRLOCK;
      case 0x39: return Key::KEY_SPACE;
      case 0x54: return Key::KEY_SYSRQ;
      case 0x0F: return Key::KEY_TAB;

      case 0x3B: return Key::KEY_F1;
      case 0x3C: return Key::KEY_F2;
      case 0x3D: return Key::KEY_F3;
      case 0x3E: return Key::KEY_F4;
      case 0x3F: return Key::KEY_F5;
      case 0x40: return Key::KEY_F6;
      case 0x41: return Key::KEY_F7;
      case 0x42: return Key::KEY_F8;
      case 0x43: return Key::KEY_F9;
      case 0x44: return Key::KEY_F10;
      case 0x57: return Key::KEY_F11;
      case 0x58: return Key::KEY_F12;

      case 0x52: return Key::KEY_0_PAD;
      case 0x4F: return Key::KEY_1_PAD;
      case 0x50: return Key::KEY_2_PAD;
      case 0x51: return Key::KEY_3_PAD;
      case 0x4B: return Key::KEY_4_PAD;
      case 0x4C: return Key::KEY_5_PAD;
      case 0x4D: return Key::KEY_6_PAD;
      case 0x47: return Key::KEY_7_PAD;
      case 0x48: return Key::KEY_8_PAD;
      case 0x49: return Key::KEY_9_PAD;
      case 0x53: return Key::KEY_DEL_PAD;
      case 0x37: return Key::KEY_PRTSCR;
      case 0x4A: return Key::KEY_MINUS_PAD;
      case 0x4E: return Key::KEY_PLUS_PAD;

      case 0x02: return Key::KEY_1;
      case 0x03: return Key::KEY_2;
      case 0x04: return Key::KEY_3;
      case 0x05: return Key::KEY_4;
      case 0x06: return Key::KEY_5;
      case 0x07: return Key::KEY_6;
      case 0x08: return Key::KEY_7;
      case 0x09: return Key::KEY_8;
      case 0x0A: return Key::KEY_9;
      case 0x0B: return Key::KEY_0;

      case 0x0C: return Key::KEY_MINUS;
      case 0x0D: return Key::KEY_EQUALS;
      case 0x1A: return Key::KEY_OPENBRACE;
      case 0x1B: return Key::KEY_CLOSEBRACE;
      case 0x27: return Key::KEY_COLON;
      case 0x28: return Key::KEY_QUOTE;
      case 0x29: return Key::KEY_TILDE;
      case 0x2B: return Key::KEY_BACKSLASH;
      case 0x33: return Key::KEY_COMMA;
      case 0x34: return Key::KEY_STOP;
      case 0x35: return Key::KEY_SLASH;

      case 0x1E: return Key::KEY_A;
      case 0x30: return Key::KEY_B;
      case 0x2E: return Key::KEY_C;
      case 0x20: return Key::KEY_D;
      case 0x12: return Key::KEY_E;
      case 0x21: return Key::KEY_F;
      case 0x22: return Key::KEY_G;
      case 0x23: return Key::KEY_H;
      case 0x17: return Key::KEY_I;
      case 0x24: return Key::KEY_J;
      case 0x25: return Key::KEY_K;
      case 0x26: return Key::KEY_L;
      case 0x32: return Key::KEY_M;
      case 0x31: return Key::KEY_N;
      case 0x18: return Key::KEY_O;
      case 0x19: return Key::KEY_P;
      case 0x10: return Key::KEY_Q;
      case 0x13: return Key::KEY_R;
      case 0x1F: return Key::KEY_S;
      case 0x14: return Key::KEY_T;
      case 0x16: return Key::KEY_U;
      case 0x2F: return Key::KEY_V;
      case 0x11: return Key::KEY_W;
      case 0x2D: return Key::KEY_X;
      case 0x15: return Key::KEY_Y;
      case 0x2C: return Key::KEY_Z;
      default: break;
    }

    return Key::KEY_UNKNOWN;
  }

public:
  KeyboardEventHandlerResult handle(uint8_t scancode) {
    const bool is_pressed = scancode >= 0x80;

    return KeyboardEventHandlerResult(e00::InputEvent(
      is_pressed ? e00::InputEvent::Type::Release : e00::InputEvent::Type::Press,
      KeyboardInputSystem(),
      key_from_lower_scancode(scancode & 0x7FU)));
  }
};

/*

        Key	     Make  Break		Key    Make  Break

        Backspace     0E    8E			F1	3B    BB
        Caps Lock     3A    BA			F2	3C    BC
        Enter	      1C    9C			F3	3D    BD
        Esc	      01    81			F4	3E    BE
        Left Alt      38    B8			F7	41    C1
        Left Ctrl     1D    9D			F5	3F    BF
        Left Shift    2A    AA			F6	40    C0
        Num Lock      45    C5			F8	42    C2
        Right Shift   36    B6			F9	43    C3
        Scroll Lock   46    C6			F10	44    C4
        Space	      39    B9			F11	57    D7
        Sys Req (AT)  54    D4			F12	58    D8
        Tab	      0F    8F

                    Keypad Keys		       Make   Break

                    Keypad 0  (Ins)		52	D2
                    Keypad 1  (End) 		4F	CF
                    Keypad 2  (Down arrow)	50	D0
                    Keypad 3  (PgDn)		51	D1
                    Keypad 4  (Left arrow)	4B	CB
                    Keypad 5			4C	CC
                    Keypad 6  (Right arrow)	4D	CD
                    Keypad 7  (Home)		47	C7
                    Keypad 8  (Up arrow)	48	C8
                    Keypad 9  (PgUp)		49	C9
                    Keypad .  (Del) 		53	D3
                    Keypad *  (PrtSc)		37	B7
                    Keypad -			4A	CA
                    Keypad +			4E	CE

               Key    Make  Break	       Key    Make  Break

                A      1E    9E 		N      31    B1
                B      30    B0 		O      18    98
                C      2E    AE 		P      19    99
                D      20    A0 		Q      10    90
                E      12    92 		R      13    93
                F      21    A1 		S      1F    9F
                G      22    A2 		T      14    94
                H      23    A3 		U      16    96
                I      17    97 		V      2F    AF
                J      24    A4 		W      11    91
                K      25    A5 		X      2D    AD
                L      26    A6 		Y      15    95
                M      32    B2 		Z      2C    AC

               Key    Make  Break	       Key    Make  Break

                1      02    82 		-      0C    8C
                2      03    83 		=      0D    8D
                3      04    84 		[      1A    9A
                4      05    85 		]      1B    9B
                5      06    86 		;      27    A7
                6      07    87 		'      28    A8
                7      08    88 		`      29    A9
                8      09    89 		\      2B    AB
                9      0A    8A 		,      33    B3
                0      0B    8B 		.      34    B4
                                                /      35    B5

 */
