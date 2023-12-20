#pragma once

#include <cstdint>
#include "Engine.hpp"

#include "KeyboardEventHandlerResult.hpp"
#include "Keys.hpp"

class KeyboardEventExtendedHandler {
  enum class ActivityState {
    NOT_ACTIVE,
    ACTIVE_LEVEL1,
  };

  ActivityState _active_state;

public:
  KeyboardEventExtendedHandler() : _active_state(ActivityState::NOT_ACTIVE) {}

  [[nodiscard]] bool is_active() const { return _active_state != ActivityState::NOT_ACTIVE; }

  static Key key_from_lower_scancode(uint8_t scancode) {
    switch (scancode) {
      case 0x2A: return Key::KEY_LSHIFT;
      case 0x36: return Key::KEY_RSHIFT;
      case 0x38: return Key::KEY_ALTGR;
      case 0x1D: return Key::KEY_RCONTROL;
      case 0x48: return Key::KEY_UP;
      case 0x4D: return Key::KEY_RIGHT;
      case 0x4B: return Key::KEY_LEFT;
      case 0x50: return Key::KEY_DOWN;
      case 0x53: return Key::KEY_DEL;
      case 0x1C: return Key::KEY_ENTER;
      case 0x4F: return Key::KEY_END;
      case 0x47: return Key::KEY_HOME;
      case 0x52: return Key::KEY_INSERT;
      case 0x51: return Key::KEY_PGDN;
      case 0x49: return Key::KEY_PGUP;
      default: break;
    }

    return Key::KEY_UNKNOWN;
  }

  KeyboardEventHandlerResult handle(uint8_t scancode) {
    switch (_active_state) {
      case ActivityState::NOT_ACTIVE:
        // If we're not active, treat 0xE0 as our activation marker
        if (scancode == 0xE0) {
          _active_state = ActivityState::ACTIVE_LEVEL1;
          return KeyboardEventHandlerResult(false);
        }
        return KeyboardEventHandlerResult(true);


      case ActivityState::ACTIVE_LEVEL1:
        _active_state = ActivityState::NOT_ACTIVE;
        return KeyboardEventHandlerResult(e00::InputEvent(
          scancode >= 0x80U ? e00::InputEvent::Type::Release
                            : e00::InputEvent::Type::Press,
          KeyboardInputSystem(),
          key_from_lower_scancode(scancode & 0x7FU),
          0));
    }

    return KeyboardEventHandlerResult(true);
  }
};

/*

        Control Keys		  Make		  Break

        Alt-PrtSc (SysReq)	  54		  D4
        Ctrl-PrtSc		  E0 37 	  E0 B7
        Enter			  E0 1C 	  E0 9C
        PrtSc			  E0 2A E0 37	  E0 B7 E0 AA
        Right Alt		  E0 38 	  E0 B8
        Right Ctrl		  E0 1D 	  E0 9D
        Shift-PrtSc		  E0 37 	  E0 B7
        /			  E0 35 	  E0 B5
        Pause			  E1 1D 45 E1 9D C5  (not typematic)
        Ctrl-Pause (Ctrl-Break)   E0 46 E0 C6	     (not typematic)

        - Keys marked as "not typematic" generate one stream of bytes
          without corresponding break scan code bytes (actually the
          break codes are part of the make code).


                        Normal Mode or
                        Shift w/Numlock
        Key		 Make	 Break	   |----- Numlock on ------.
                                              Make	    Break
        Del		 E0 53	 E0 D3	   E0 2A E0 53	 E0 D3 E0 AA
        Down arrow	 E0 50	 E0 D0	   E0 2A E0 50	 E0 D0 E0 AA
        End		 E0 4F	 E0 CF	   E0 2A E0 4F	 E0 CF E0 AA
        Home		 E0 47	 E0 C7	   E0 2A E0 47	 E0 C7 E0 AA
        Ins		 E0 52	 E0 D2	   E0 2A E0 52	 E0 D2 E0 AA
        Left arrow	 E0 4B	 E0 CB	   E0 2A E0 4B	 E0 CB E0 AA
        PgDn		 E0 51	 E0 D1	   E0 2A E0 51	 E0 D1 E0 AA
        PgUp		 E0 49	 E0 C9	   E0 2A E0 49	 E0 C9 E0 AA
        Right arrow	 E0 4D	 E0 CD	   E0 2A E0 4D	 E0 CD E0 AA
        Up arrow	 E0 48	 E0 C8	   E0 2A E0 48	 E0 C8 E0 AA

        Key	      |--Left Shift Pressed--.	  |--Right Shift Pressed--.
                         Make	       Break	      Make	    Break
        Del	      E0 AA E0 53   E0 D3 E0 2A    E0 B6 E0 53	 E0 D3 E0 36
        Down arrow    E0 AA E0 50   E0 D0 E0 2A    E0 B6 E0 50	 E0 D0 E0 36
        End	      E0 AA E0 4F   E0 CF E0 2A    E0 B6 E0 4F	 E0 CF E0 36
        Home	      E0 AA E0 47   E0 C7 E0 2A    E0 B6 E0 47	 E0 C7 E0 36
        Ins	      E0 AA E0 52   E0 D2 E0 2A    E0 B6 E0 52	 E0 D2 E0 36
        Left arrow    E0 AA E0 4B   E0 CB E0 2A    E0 B6 E0 4B	 E0 CB E0 36
        PgDn	      E0 AA E0 51   E0 D1 E0 2A    E0 B6 E0 51	 E0 D1 E0 36
        PgUp	      E0 AA E0 49   E0 C9 E0 2A    E0 B6 E0 49	 E0 C9 E0 36
        Right arrow   E0 AA E0 4D   E0 CD E0 2A    E0 B6 E0 4D	 E0 CD E0 36
        Up arrow      E0 AA E0 48   E0 C8 E0 2A    E0 B6 E0 48	 E0 C8 E0 36
        /	      E0 AA E0 35   E0 B5 E0 2A    E0 B6 E0 35	 E0 B5 E0 36
 */
