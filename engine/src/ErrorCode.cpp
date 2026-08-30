#include <Engine/Detail/ErrorCode.hpp>


namespace e00 {

// ------------------------
// error_category base
// ------------------------
error_category::~error_category() = default;

// default_error_condition: same semantics as std::error_category
error_condition error_category::default_error_condition(int i) const noexcept {
  // Default: condition with same value in this category
  return error_condition(i, *this);
}

// equivalent(int, error_condition):
// Equivalent to default_error_condition(i) == cond
bool error_category::equivalent(int i,
                                const error_condition &cond) const noexcept {
  return default_error_condition(i) == cond;
}

// equivalent(error_code, int):
// Equivalent to *this == code.category() && code.value() == i
bool error_category::equivalent(const error_code &code,
                                int i) const noexcept {
  return *this == code.category() && code.value() == i;
}

// ------------------------
// concrete categories
// ------------------------
class generic_error_category : public error_category {
public:
  const char *name() const noexcept override {
    return "generic";
  }

  std::string message(int ev) const override {
    switch (static_cast<errc>(ev)) {
      case errc::success: return "success";
      case errc::invalid_argument: return "invalid argument";
      case errc::not_supported: return "operation not supported";
      case errc::io_error: return "I/O error";
      case errc::timed_out: return "timed out";
      default: return "unknown generic error";
    }
  }

  error_condition default_error_condition(int i) const noexcept override {
    // For now, just map directly; you can add richer mapping later.
    return error_condition(i, *this);
  }
};

class system_error_category : public error_category {
public:
  const char *name() const noexcept override {
    return "system";
  }

  std::string message(int ev) const override {
    // You can hook into OS error strings here if you want.
    switch (static_cast<errc>(ev)) {
      case errc::success: return "success";
      case errc::invalid_argument: return "invalid argument";
      case errc::not_supported: return "operation not supported";
      case errc::io_error: return "I/O error";
      case errc::timed_out: return "timed out";
      default: return "unknown system error";
    }
  }

  error_condition default_error_condition(int i) const noexcept override {
    // Map system errors to generic conditions if you want;
    // for now, direct mapping.
    return error_condition(i, generic_category());
  }
};

// ------------------------
// category singletons
// ------------------------
const error_category &generic_category() noexcept {
  static generic_error_category cat;
  return cat;
}

const error_category &system_category() noexcept {
  static system_error_category cat;
  return cat;
}

// ------------------------
// error_code::default_error_condition
// ------------------------
error_condition error_code::default_error_condition() const noexcept { return category().default_error_condition(value()); }
}// namespace e00
