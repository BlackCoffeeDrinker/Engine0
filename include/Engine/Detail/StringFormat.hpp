#pragma once

#include <algorithm>
#include <limits>
#include <type_traits>
#include <string>
#include <string_view>

#include "magic_enum.hpp"

namespace e00::fmt_lite {
const char digit_pairs[201] = {
  "00010203040506070809"
  "10111213141516171819"
  "20212223242526272829"
  "30313233343536373839"
  "40414243444546474849"
  "50515253545556575859"
  "60616263646566676869"
  "70717273747576777879"
  "80818283848586878889"
  "90919293949596979899"
};


template<typename T>
constexpr unsigned int num_digits(T number) {
  unsigned int digits = 0;
  if (number < 0)
    digits = 1;
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}

template<>
constexpr unsigned int num_digits(int x) {
  if (x < 0)
    return num_digits(-x) + 1;

  if (x >= 10000) {
    if (x >= 10000000) {
      if (x >= 100000000) {
        if (x >= 1000000000)
          return 10;
        return 9;
      }

      return 8;
    }

    if (x >= 100000) {
      if (x >= 1000000)
        return 7;
      return 6;
    }

    return 5;
  }

  if (x >= 100) {
    if (x >= 1000)
      return 4;
    return 3;
  }

  if (x >= 10)
    return 2;
  return 1;
}

template<>
constexpr unsigned int num_digits(char x) {
  if (x < 0)
    return num_digits(-x) + 1;
  if (x >= 100)
    return 3;
  if (x >= 10)
    return 2;
  return 1;
}

template<typename T>
struct formatter { formatter() = delete; };

template<typename T>
struct integer_formatter {
  template<typename Context>
  void parse(Context &ctx, T value) {
    char buffer[num_digits(std::numeric_limits<T>::max())];

    if (value < 0) {
      *ctx._out++ = '-';
      parse(ctx, static_cast<T>(-value));
      return;
    }

    const auto size = num_digits(value);
    char *c = buffer + size - 1;

    while (value >= 100) {
      auto pos = value % 100;
      value /= 100;
      *(short *)(c - 1) = *(short *)(digit_pairs + 2 * pos);
      c -= 2;
    }

    while (value > 0) {
      *c-- = static_cast<char>('0' + static_cast<char>(value % 10));
      value /= 10;
    }

    std::copy(buffer, buffer + size, ctx._out);
  }
};

template<>
struct formatter<int> : integer_formatter<int> { formatter() = default; };
template<>
struct formatter<char> : integer_formatter<char> {
  formatter() = default;
  template<typename Context>
  void parse(Context& ctx, const char& v) {
    *ctx._out = v;
    ctx._out++;
  }
};
template<>
struct formatter<unsigned int> : integer_formatter<unsigned int> { formatter() = default; };
template<>
struct formatter<unsigned long> : integer_formatter<unsigned long> { formatter() = default; };
template<>
struct formatter<long long> : integer_formatter<long long> { formatter() = default; };
template<>
struct formatter<long int> : integer_formatter<long int> { formatter() = default; };
template<>
struct formatter<bool> {
  formatter() = default;

  template<typename Context>
  void parse(Context &ctx, const bool &v) {
    std::string bool_value;

    if (v) {
      bool_value = "True";
    } else {
      bool_value = "False";
    }

    std::copy(bool_value.begin(), bool_value.end(), ctx._out);
  }
};
template<>
struct formatter<std::string_view> {
  formatter() = default;

  template<typename Context>
  void parse(Context &ctx, const std::string_view &v) {
    std::copy(v.begin(), v.end(), ctx._out);
  }
};
template<>
struct formatter<const void *> {
  formatter() = default;

  template<typename Context>
  void parse(Context &ctx, const void *value) {
    formatter<ptrdiff_t>().parse(ctx, reinterpret_cast<ptrdiff_t>(value));
  }
};

namespace internal {
  template<typename OutputIt>
  struct ArgFormatter {
    OutputIt _out;
    constexpr ArgFormatter(OutputIt it) : _out(it) {}

    template<typename T>
    constexpr typename std::enable_if<std::is_constructible<formatter<T>>::value, void>::type operator()(const T &i) {
      formatter<T>().parse(*this, i);
    }

    constexpr void operator()(long double) {}
  };

  enum class arg_type {
    none_type,
    int_type,
    uint_type,
    ulong_type,
    long_long_type,
    ulong_long_type,
    bool_type,
    char_type,
    float_type,
    double_type,
    long_double_type,
    cstring_type,
    string_view_type,
    pointer_type,
  };

  // Holds all types of possible arguments
  class format_arg {
    union {
      const int int_value;
      const unsigned uint_value;
      const long long long_long_value;
      const unsigned long ulong;
      const unsigned long long ulong_long_value;
      const bool bool_value;
      const char char_value;
      const float float_value;
      const double double_value;
      const long double long_double_value;
      const void *pointer;
      const std::string_view str_view;
    };

    const arg_type type;

  public:
    format_arg() : type(arg_type::none_type) {}
    explicit format_arg(std::string_view string_view) : str_view(string_view), type(arg_type::string_view_type) {}
    explicit format_arg(const std::string& str) : str_view(str), type(arg_type::string_view_type) {}
    explicit format_arg(const char *string_value) : str_view(std::string_view(string_value)), type(arg_type::cstring_type) {}
    explicit format_arg(const void *ptr) : pointer(ptr), type(arg_type::pointer_type) {}
    explicit format_arg(long double ldv) : long_double_value(ldv), type(arg_type::long_double_type) {}
    explicit format_arg(double dv) : double_value(dv), type(arg_type::double_type) {}
    explicit format_arg(float fv) : float_value(fv), type(arg_type::float_type) {}
    explicit format_arg(char cv) : char_value(cv), type(arg_type::char_type) {}
    explicit format_arg(bool bv) : bool_value(bv), type(arg_type::bool_type) {}
    explicit format_arg(unsigned long long ullv) : ulong_long_value(ullv), type(arg_type::ulong_long_type) {}
    explicit format_arg(long long llv) : long_long_value(llv), type(arg_type::long_long_type) {}
    explicit format_arg(unsigned uiv) : uint_value(uiv), type(arg_type::uint_type) {}
    explicit format_arg(int iv) : int_value(iv), type(arg_type::int_type) {}
    explicit format_arg(unsigned long ul) : ulong(ul), type(arg_type::ulong_type) {}

    // Visits an arg by contained_type
    template<typename Visitor>
    constexpr void visit_arg(Visitor &&vis) const {
      switch (type) {
        case arg_type::none_type: break;
        case arg_type::string_view_type: vis(str_view); break;
        case arg_type::ulong_type: vis(ulong); break;
        case arg_type::cstring_type: vis(str_view); break;
        case arg_type::pointer_type: vis(pointer); break;
        case arg_type::long_double_type: vis(long_double_value); break;
        case arg_type::double_type: vis(double_value); break;
        case arg_type::float_type: vis(float_value); break;
        case arg_type::char_type: vis(char_value); break;
        case arg_type::bool_type: vis(bool_value); break;
        case arg_type::ulong_long_type: vis(ulong_long_value); break;
        case arg_type::long_long_type: vis(long_long_value); break;
        case arg_type::uint_type: vis(uint_value); break;
        case arg_type::int_type: vis(int_value); break;
      }
    }
  };

  template<typename T>
  typename std::enable_if<!std::is_enum<T>::value, format_arg>::type make_arg(const T &arg) {
    return format_arg(arg);
  }

  template<typename T>
  typename std::enable_if<std::is_enum<T>::value, format_arg>::type make_arg(const T &arg) {
    return format_arg(magic_enum::enum_name<T>(arg));
  }

  // constexpr find
  template<typename T, typename Ptr = const T *>
  constexpr bool find(Ptr first, Ptr last, T value, Ptr &out) {
    for (out = first; out != last; ++out) {
      if (*out == value) {
        return true;
      }
    }
    return false;
  }

  template<typename OutputIt, size_t N>
  void HandleFormattingArguments(
    std::string_view fmt_options,
    size_t& next_unnumbered_args,
    OutputIt out,
    std::array<internal::format_arg, N>& fmt_args) {

    // Todo: split this into 2 methods ?

    auto arg_fmt = internal::ArgFormatter(out);
    bool has_found_index = false;
    size_t index = 0;

    auto current_p = fmt_options.begin();
    const auto end_p = fmt_options.end();

    // Parse the index
    while (current_p != end_p) {
      // did we find the "options" marker ?
      if (*current_p == ':') {
        // if so, increment and get out of here
        current_p++;
        break;
      }

      // parse the index
      has_found_index = true;
      index = index * 10 + unsigned(*current_p - '0');
      current_p++;
    }

    // Parse the options
    while (current_p != end_p) {
      // TODO: Parse options
      current_p++;
    }

    // Ask the formatter, configured with options, to format this argument
    if (has_found_index) {
      if (index < N) {
        fmt_args.at(index).visit_arg(arg_fmt);
      }
    } else {
      fmt_args.at(next_unnumbered_args++).visit_arg(arg_fmt);
    }
  }

}// namespace internal

template<typename... Args>
std::string format(std::string_view fmt, const Args& ...args) {
  if constexpr (sizeof...(Args) == 0) {
    return std::string(fmt);
  }

  std::string output;

  // Output is at least as long as the input
  output.reserve(fmt.size());

  // Make the format_args
  std::array<internal::format_arg, sizeof...(Args)> fmt_args = { internal::make_arg(args)... };

  auto out = std::back_inserter(output);

  size_t current_pos = 0;
  size_t next_unnumbered_arg = 0;

  while (current_pos != std::string_view::npos) {
    const auto next_open = fmt.find('{', current_pos);

    // Append everything before the next_open
    if (next_open - current_pos > 0) {
      const auto before_open = fmt.substr(current_pos, next_open - current_pos);
      std::copy(before_open.begin(), before_open.end(), out);
    }

    // If we hit the end then we don't need to loop anymore
    if (next_open == std::string_view::npos) {
      break;
    }

    // Find the }
    const auto closing = fmt.find('}', next_open + 1);
    if (closing == std::string_view::npos) {
      // something's wrong!
      return {};
    }

    // Do we have something between open and close ?
    if (closing - next_open > 1) {
      const auto fmt_options = fmt.substr(next_open + 1, closing - next_open - 1);
      internal::HandleFormattingArguments(fmt_options, next_unnumbered_arg, out, fmt_args);
    } else {
      // If we don't have any arguments, take the next unnumbered arg and copy it in there
      internal::HandleFormattingArguments({}, next_unnumbered_arg, out, fmt_args);
    }

    current_pos = closing + 1;
  }


  return output;
}
}// namespace e00::fmt_lite
