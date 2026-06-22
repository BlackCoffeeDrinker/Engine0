
#pragma once

#include "Platform.hpp"

#include <SDL3/SDL_events.h>

namespace platform {

/**
 * Class SDL_KeyboardSystem
 */
class SDL_KeyboardSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override {
    return "Keyboard";
  }

  [[nodiscard]] std::string name(uint32_t value) const override;
};

const SDL_KeyboardSystem& GetKeyboardSystem();
e00::InputEvent MakeSDLKey(const SDL_KeyboardEvent &key_event);

}// namespace platform
