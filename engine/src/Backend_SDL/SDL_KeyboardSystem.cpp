
#include "SDL_KeyboardSystem.h"

#include <SDL2/SDL_keyboard.h>

namespace {
constexpr platform::SDL_KeyboardSystem keyboard_system{};
}

namespace platform {
std::string SDL_KeyboardSystem::name(uint32_t value) const {
  return {SDL_GetKeyName(static_cast<SDL_Keycode>(value))};
}

const SDL_KeyboardSystem &GetKeyboardSystem() {
  return keyboard_system;
}

e00::InputEvent MakeSDLKey(const SDL_KeyboardEvent &key_event) {
  return {keyboard_system, key_event.key};
}
}// namespace platform
