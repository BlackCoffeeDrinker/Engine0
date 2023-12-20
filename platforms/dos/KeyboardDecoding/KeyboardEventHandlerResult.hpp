#pragma once

#include "Engine.hpp"

struct KeyboardEventHandlerResult {
  const bool should_continue_processing;
  const bool has_event_data;
  const e00::InputEvent event;

  explicit KeyboardEventHandlerResult(bool s) : should_continue_processing(s), has_event_data(false), event() {}

  explicit KeyboardEventHandlerResult(e00::InputEvent event_data) : should_continue_processing(false), has_event_data(true), event(event_data) {}
};
