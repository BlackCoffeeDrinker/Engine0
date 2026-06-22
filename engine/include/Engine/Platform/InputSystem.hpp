#pragma once

namespace e00 {
using input_value_t = uint16_t;

/**
 *
 */
struct InputSystem {
  virtual ~InputSystem() = default;
  /**
   * obtains the name of the input system
   * (eg. Mouse, Keyboard...)
   *
   * @return a string specifying the name of the input system.
   */
  [[nodiscard]] virtual std::string name() const = 0;

  /**
   * Returns a string describing the given error condition for the
   * input event represented by *this
   *
   * @param value specifies the input event to describe
   * @return A string describing the given input event.
   */
  [[nodiscard]] virtual std::string name(input_value_t value) const = 0;
};


const InputSystem &UnknownInputSystem() noexcept;

}// namespace e00
