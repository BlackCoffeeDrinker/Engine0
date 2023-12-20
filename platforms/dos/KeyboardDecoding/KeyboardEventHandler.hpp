#pragma once

#include <dpmi.h>

#include "KeyboardEventHandlerResult.hpp"
#include "KeyboardEventPauseHandler.hpp"
#include "KeyboardEventExtendedHandler.hpp"
#include "KeyboardEventNormalHandler.hpp"

#include "Engine/InputEvent.hpp"

class KeyboardEventHandler {
  KeyboardEventPauseHandler _pause_handler;
  KeyboardEventExtendedHandler _extended_handler;
  KeyboardEventNormalHandler _normal_handler;

  _go32_dpmi_seginfo keyboardOldISR, keyboardNewISR;
  int next_key;

public:
  KeyboardEventHandler() noexcept;

  ~KeyboardEventHandler() noexcept;

  e00::InputEvent event() noexcept;
};
