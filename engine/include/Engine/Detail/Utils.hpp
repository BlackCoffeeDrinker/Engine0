
#pragma once

#include <Engine/Detail/ErrorCode.hpp>
#include <Engine/Math/Color.hpp>
#include <Engine/Math/Vec2D.hpp>

namespace e00 {

enum class ParseError {
  ok = 0,
  invalid_argument,
  result_out_of_range
};

struct ParseCharsResults {
  const char *ptr;
  ParseError ec;

  constexpr explicit operator bool() const noexcept {
    return ec == ParseError::ok;
  }
};

template<typename T>
ParseCharsResults ParseChars(const std::string_view &view, T &out) {
  static_assert(std::is_integral_v<T>, "ParseChars only supports integral types.");
  static_assert(!std::is_same_v<T, bool>, "ParseChars does not support bool.");

  const char *curr = view.data();
  const char *const end = view.data() + view.size();

  if (curr == end) {
    return {.ptr = view.data(), .ec = ParseError::invalid_argument};
  }

  bool is_negative = false;
  if constexpr (std::numeric_limits<T>::is_signed) {
    if (*curr == '-') {
      is_negative = true;
      ++curr;
    }
  }

  const char *const digits_start = curr;
  bool overflow = false;
  T accum = 0;

  constexpr T max_val = std::numeric_limits<T>::max();
  constexpr T min_val = std::numeric_limits<T>::min();

  for (; curr != end && *curr >= '0' && *curr <= '9'; ++curr) {
    int digit = *curr - '0';

    if (overflow) {
      // Standard std::from_chars behavior: consume remaining digits
      // so ptr points to the end of the number matching pattern
      continue;
    }

    if constexpr (std::numeric_limits<T>::is_signed) {
      if (is_negative) {
        if (accum < (min_val + digit) / 10) {
          overflow = true;
        } else {
          accum = accum * 10 - digit;
        }
      } else {
        if (accum > (max_val - digit) / 10) {
          overflow = true;
        } else {
          accum = accum * 10 + digit;
        }
      }
    } else {
      if (accum > (max_val - digit) / 10) {
        overflow = true;
      } else {
        accum = static_cast<T>(accum * 10 + digit);
      }
    }
  }

  // Rejects if no digits were parsed (e.g., empty, "+123", or "-")
  if (curr == digits_start) {
    return {.ptr = view.data(), .ec = ParseError::invalid_argument};
  }

  if (overflow) {
    return {.ptr = curr, .ec = ParseError::result_out_of_range};
  }

  out = accum;
  return {.ptr = curr, .ec = ParseError::ok};
}

bool ParseNumber(const std::string_view &input, int &output);
bool ParseBool(const std::string_view &value, bool defaultValue);
bool ParseWorldPoint(const std::string_view &value, WorldPosition &out);
bool ParseRGB(const std::string_view &sv, Color &color_out);

template<typename RealType>
error_code ToSize(const std::string_view &str, size_t &size) {
  const auto value = ParseChars(str, size);

  if (value.ec == ParseError::invalid_argument) {
    return make_error_code(errc::invalid_argument);
  }
  if (value.ec == ParseError::result_out_of_range) {
    return make_error_code(errc::result_out_of_range);
  }
  // Enforce full string consumption (reject partial matches like "123abc")
  if (value.ptr != str.data() + str.size()) {
    return make_error_code(errc::invalid_argument);
  }

  if (size > std::numeric_limits<RealType>::max()) {
    return make_error_code(errc::invalid_argument);
  }
  if (size < std::numeric_limits<RealType>::min()) {
    return make_error_code(errc::invalid_argument);
  }

  return {};
}

}// namespace e00
