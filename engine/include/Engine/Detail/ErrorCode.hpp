
#pragma once

#include <compare>
#include <string>
#include <type_traits>

namespace e00 {
class error_code;
class error_condition;
class system_error;

template<typename Tp>
struct is_error_code_enum : std::false_type {};
template<typename Tp>
struct is_error_condition_enum : std::false_type {};

// ------------------------------------------------------------
// e00::errc — same numeric values as std::errc for compatibility
// ------------------------------------------------------------
enum class errc : int {
  success = 0,
  no_such_file_or_directory = 2,
  invalid_argument = 22,
  not_enough_memory = 12,
  not_supported = 95,
  io_error = 5,
  result_out_of_range = 34,
  timed_out = 110,
  value_too_large = 132,
  function_not_supported = 40,
};

template<>
struct is_error_condition_enum<errc> : std::true_type {};

// C++17 stuff
template<typename Tp>
inline constexpr bool is_error_code_enum_v = is_error_code_enum<Tp>::value;
template<typename Tp>
inline constexpr bool is_error_condition_enum_v = is_error_condition_enum<Tp>::value;


class error_category {
public:
  constexpr error_category() noexcept = default;
  virtual ~error_category();

  error_category(const error_category &) = delete;
  error_category &operator=(const error_category &) = delete;

  /// A string that identifies the error category.
  [[nodiscard]] virtual const char *name() const noexcept = 0;
  [[nodiscard]] virtual std::string message(int) const = 0;
  [[nodiscard]] virtual error_condition default_error_condition(int i) const noexcept;
  [[nodiscard]] virtual bool equivalent(int i, const error_condition &cond) const noexcept;
  [[nodiscard]] virtual bool equivalent(const error_code &code, int i) const noexcept;
  [[nodiscard]] bool operator==(const error_category &other) const noexcept { return this == &other; }

  /// Ordered comparison that defines a total order for error categories.
#if __cpp_lib_three_way_comparison
  [[nodiscard]]
  std::strong_ordering operator<=>(const error_category &rhs) const noexcept { return std::compare_three_way()(this, &rhs); }
#else
  bool
  operator<(const error_category &other) const noexcept { return less<const error_category *>()(this, &other); }

  bool
  operator!=(const error_category &other) const noexcept { return this != &other; }
#endif
};

[[__nodiscard__, __gnu__::__const__]] const error_category &generic_category() noexcept;
[[__nodiscard__, __gnu__::__const__]] const error_category &system_category() noexcept;

// ------------------------------------------------------------
// e00::error_code — drop‑in replacement for e00::error_code
// ------------------------------------------------------------
class error_code {
  template<typename ErrorCodeEnum>
  using Check = std::enable_if_t<is_error_code_enum<ErrorCodeEnum>::value>;

  int _value;
  const error_category *_cat;

public:
  constexpr error_code() noexcept : _value(0), _cat(&system_category()) {}
  error_code(int v, const error_category &cat) noexcept : _value(v), _cat(&cat) {}
  error_code(const error_code &) = default;
  error_code &operator=(const error_code &) = default;

  void assign(int v, const error_category &cat) noexcept {
    _value = v;
    _cat = &cat;
  }

  void clear() noexcept { assign(0, system_category()); }
  [[nodiscard]] int value() const noexcept { return _value; }
  [[nodiscard]] const error_category &category() const noexcept { return *_cat; }
  error_condition default_error_condition() const noexcept;
  [[nodiscard]] std::string message() const { return category().message(value()); }
  [[nodiscard]] explicit operator bool() const noexcept { return _value != 0; }
};

class error_condition {
  template<typename ErrorConditionEnum>
  using Check = std::enable_if_t<is_error_condition_enum<ErrorConditionEnum>::value>;

  int _value;
  const error_category *_cat;

public:
  /// Initialize with a zero (no error) value and the generic category.
  error_condition() noexcept : _value(0), _cat(&generic_category()) {}
  error_condition(int v, const error_category &cat) noexcept : _value(v), _cat(&cat) {}
  error_condition(const error_condition &) = default;
  error_condition &operator=(const error_condition &) = default;

  void assign(int v, const error_category &cat) noexcept {
    _value = v;
    _cat = &cat;
  }
  void clear() noexcept { assign(0, generic_category()); }
  [[nodiscard]] int value() const noexcept { return _value; }
  [[nodiscard]] const error_category &category() const noexcept { return *_cat; }
  [[nodiscard]] std::string message() const { return category().message(value()); }
  [[nodiscard]] explicit operator bool() const noexcept { return _value != 0; }


  friend bool operator==(const error_condition &a, const error_condition &b) noexcept {
    return a._value == b._value && a._cat == b._cat;
  }

  friend bool operator!=(const error_condition &a, const error_condition &b) noexcept {
    return !(a == b);
  }
};

inline error_code make_error_code(errc e) noexcept { return error_code(static_cast<int>(e), generic_category()); }

}// namespace e00
