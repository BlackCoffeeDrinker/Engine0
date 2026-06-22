#pragma once

#include "Platform.hpp"
#include "Engine/Platform/InputSystem.hpp"

#include <string>

#ifdef __OBJC__
@class NSEvent;
#else
class NSEvent;
#endif

namespace platform {

/**
 * Apple/macOS keyboard input system.
 *
 * The event id is the hardware-independent macOS virtual key code from NSEvent::keyCode.
 */
class Apple_KeyboardSystem final : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override {
    return "Keyboard";
  }

  [[nodiscard]] std::string name(e00::input_value_t value) const override;
};

// ... existing code ...
const Apple_KeyboardSystem &GetKeyboardSystem();

#ifdef __OBJC__
e00::InputEvent MakeAppleKey(NSEvent *event, e00::InputEvent::Type type);
#endif

}// namespace platform
