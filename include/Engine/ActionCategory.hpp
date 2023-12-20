#pragma once

namespace e00 {
template<typename T>
struct IsActionTypeEnum : std::false_type {};

/**
 * e00::ActionCategory serves as the base class for specific action category types
 * The objects of action category classes are treated as singletons, passed by reference.
 */
struct ActionCategory {
  constexpr ActionCategory() noexcept = default;
  ActionCategory(const ActionCategory &) = delete;
  virtual ~ActionCategory() noexcept = default;

  [[nodiscard]] virtual std::string_view name() const noexcept = 0;
  [[nodiscard]] virtual std::string_view message(uint32_t binding) const = 0;

  friend bool operator==(const ActionCategory &left, const ActionCategory &right) noexcept;
  friend bool operator!=(const ActionCategory &left, const ActionCategory &right) noexcept;
  friend bool operator<(const ActionCategory &left, const ActionCategory &right) noexcept;

  ActionCategory &operator=(const ActionCategory &) = delete;
};

inline bool operator==(const ActionCategory &left, const ActionCategory &right) noexcept {
  return &left == &right;
}

inline bool operator!=(const ActionCategory &left, const ActionCategory &right) noexcept {
  return !(left == right);
}

inline bool operator<(const ActionCategory &left, const ActionCategory &right) noexcept {
  return &left < &right;
}

}