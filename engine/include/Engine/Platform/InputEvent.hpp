#pragma once

#include <string>

#include <Engine/Platform/InputSystem.hpp>

namespace e00 {
/**
 *
 */
class InputEvent {
public:
  using axis_t = int16_t;

  enum class Type : uint8_t {
    KeyUp,  // Keyboard, Mouse buttons, Joystick buttons, Gamepad buttons (value has key code, platform-dependent)
    KeyDown,// Keyboard, Mouse buttons, Joystick buttons, Gamepad buttons (value has key code, platform-dependent)
    Axis,   // Mouse movement, Joystick movement, Gamepad DPad/Analog Sticks (value has axis, axis_delta has value)
    Unknown
  };

private:
  const InputSystem *_input_system;
  Type _type = Type::Unknown;
  input_value_t _value = 0;
  axis_t _axis_delta = 0;

public:
  InputEvent() : _input_system(&UnknownInputSystem()), _type(Type::Unknown) {}

  InputEvent(const InputSystem &source_, Type type_, input_value_t event_id_, axis_t delta_)
      : _input_system(&source_), _type(type_), _value(event_id_), _axis_delta(delta_) {}

  InputEvent(const InputSystem &source_, Type type_, input_value_t event_id_)
      : _input_system(&source_), _type(type_), _value(event_id_) {}

  InputEvent(const InputSystem &source_, input_value_t event_id_, axis_t delta_)
      : InputEvent(source_, Type::Axis, event_id_, delta_) {}

  InputEvent(const InputSystem &source_, input_value_t event_id_)
      : InputEvent(source_, Type::KeyDown, event_id_) {}

  explicit operator bool() const noexcept {
    return _input_system != &UnknownInputSystem() && _type != Type::Unknown;
  }

  friend bool operator==(const InputEvent &left, const InputEvent &right) noexcept {
    return (left._input_system == right._input_system) &&
           (left._type == right._type) &&
           (left._value == right._value);
  }

  friend bool operator!=(const InputEvent &left, const InputEvent &right) noexcept {
    return !(left == right);
  }

  void assign(Type type, input_value_t value, const InputSystem &system) {
    _type = type;
    _value = value;
    _axis_delta = 0;
    _input_system = &system;
  }

  void assign_axis(input_value_t value, axis_t delta, const InputSystem &system) {
    _type = Type::Axis;
    _value = value;
    _axis_delta = delta;
    _input_system = &system;
  }

  void clear() {
    _type = Type::Unknown;
    _axis_delta = 0;
    assign(Type::Unknown, 0, UnknownInputSystem());
  }

  [[nodiscard]] auto type() const noexcept { return _type; }
  [[nodiscard]] auto id() const noexcept { return _value; }
  [[nodiscard]] auto axis_delta() const noexcept { return _axis_delta; }
  [[nodiscard]] auto &input_system() const noexcept { return *_input_system; }
  [[nodiscard]] auto message() const noexcept { return input_system().name(_value); }

  friend bool operator<(const InputEvent &left, const InputEvent &right) noexcept {
    return left.hash() < right.hash();
  }

  [[nodiscard]] size_t hash() const noexcept {
    const size_t part1 = static_cast<size_t>(_type) << 24;
    const size_t part2 = (reinterpret_cast<uintptr_t>(_input_system) >> 4) & 0xFFFF00;
    const size_t part3 = _value;
    return part1 | part2 | part3;
  }

  [[nodiscard]] bool is(const InputSystem &input_system, input_value_t value) const noexcept {
    return _input_system == &input_system && _value == value;
  }

  template<typename T>
  [[nodiscard]] std::enable_if_t<std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, input_value_t>, bool> is(const InputSystem &input_system, T value) const noexcept {
    static_assert(std::is_enum_v<T>);
    static_assert(std::is_same_v<std::underlying_type_t<T>, input_value_t>);
    return is(input_system, static_cast<input_value_t>(value));
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
