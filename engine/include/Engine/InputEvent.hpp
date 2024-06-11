#pragma once

#include <string>

namespace e00 {

/**
 *
 */
class InputEvent {
  const InputSystem *_input_system;
  uint16_t _value = 0;
  int16_t _axis_delta = 0;

public:
  InputEvent() : _input_system(&UnknownInputSystem()) {}

  InputEvent(const InputSystem &source_, uint16_t event_id_, int16_t delta_)
    : _input_system(&source_), _value(event_id_), _axis_delta(delta_) {}

  InputEvent(const InputSystem &source_, uint16_t event_id_)
    : _input_system(&source_), _value(event_id_) {}

  explicit operator bool() const noexcept { return _input_system != &UnknownInputSystem(); }

  friend bool operator==(const InputEvent &left, const InputEvent &right) noexcept {
    return (left._input_system == right._input_system)
           && (left._value == right._value);
  }

  friend bool operator!=(const InputEvent &left, const InputEvent &right) noexcept {
    return (left._input_system != right._input_system)
           || (left._value != right._value);
  }

  void assign(uint16_t value, const InputSystem &system) {
    _value = value;
    _input_system = &system;
  }

  void clear() {
    assign(0, UnknownInputSystem());
  }

  [[nodiscard]] auto id() const noexcept { return _value; }
  [[nodiscard]] auto axis_delta() const noexcept { return _axis_delta; }
  [[nodiscard]] auto &input_system() const noexcept { return *_input_system; }
  [[nodiscard]] auto message() const noexcept { return input_system().name(_value); }

  friend bool operator<(const InputEvent &left, const InputEvent &right) noexcept {
    return left.hash() < right.hash();
  }

  [[nodiscard]] size_t hash() const noexcept {
    const size_t part2 = reinterpret_cast<intptr_t>(_input_system) << 16;
    const size_t part3 = _value;
    return 0 | part2 | part3;
  }
};
}// namespace e00

namespace std {
template<>
struct hash<e00::InputEvent> {
  size_t operator()(e00::InputEvent event) const noexcept {
    return event.hash();
  }
};
}// namespace std
