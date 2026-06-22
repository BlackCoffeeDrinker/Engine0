#include "tests.hpp"

#include <iostream>

TEST_CASE("Source location shows actual source") {
  e00::source_location r = e00::source_location::current();

  std::string r_filename = r.file_name();
  REQUIRE(r_filename == __FILE__);
  REQUIRE(r.line() == 6);
}

const char *boolstr(bool value) {
  return value ? "true" : "false";
}
const char *round_style_str(std::float_round_style style) {
  switch (style) {
    case std::float_round_style::round_indeterminate:
      return "round_indeterminate";
    case std::float_round_style::round_toward_zero:
      return "round_toward_zero";
    case std::float_round_style::round_to_nearest:
      return "round_to_nearest";
    case std::float_round_style::round_toward_infinity:
      return "round_toward_infinity";
    case std::float_round_style::round_toward_neg_infinity:
      return "round_toward_neg_infinity";
    default:
      return "unknown";
  }
}

template<typename T>
const char *strinify(T value) {
  static char buffer[1024];
  auto string = std::to_string(value);

  strncpy(buffer, string.c_str(), sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';
  return buffer;
}

template<>
const char *strinify<bool>(bool value) {
  return boolstr(value);
}


template<typename T>
void print_for_type(std::string_view type) {
  printf("template<> struct numeric_limits<%s> {\n", type.data());
  printf("  static constexpr bool is_specialized = %s;\n", boolstr(std::numeric_limits<T>::is_specialized));
  printf("  static constexpr bool is_signed = %s;\n", boolstr(std::numeric_limits<T>::is_signed));
  printf("  static constexpr bool is_integer = %s;\n", boolstr(std::numeric_limits<T>::is_integer));
  printf("  static constexpr bool is_exact = %s;\n", boolstr(std::numeric_limits<T>::is_exact));
  printf("  static constexpr bool has_infinity = %s;\n", boolstr(std::numeric_limits<T>::has_infinity));
  printf("  static constexpr bool has_quiet_NaN = %s;\n", boolstr(std::numeric_limits<T>::has_quiet_NaN));
  printf("  static constexpr bool has_signaling_NaN = %s;\n", boolstr(std::numeric_limits<T>::has_signaling_NaN));
  printf("  static constexpr bool has_denorm = %s;\n", boolstr(std::numeric_limits<T>::has_denorm));
  printf("  static constexpr bool has_denorm_loss = %s;\n", boolstr(std::numeric_limits<T>::has_denorm_loss));
  printf("  static constexpr float_round_style round_style = %s;\n", round_style_str(std::numeric_limits<T>::round_style));
  printf("  static constexpr bool is_iec559 = %s;\n", boolstr(std::numeric_limits<T>::is_iec559));
  printf("  static constexpr bool is_bounded = %s;\n", boolstr(std::numeric_limits<T>::is_bounded));
  printf("  static constexpr bool is_modulo = %s;\n", boolstr(std::numeric_limits<T>::is_modulo));
  printf("  static constexpr int digits = %d;\n", std::numeric_limits<T>::digits);
  printf("  static constexpr int digits10 = %d;\n", std::numeric_limits<T>::digits10);
  printf("  static constexpr int max_digits10 = %d;\n", std::numeric_limits<T>::max_digits10);
  printf("  static constexpr int radix = %d;\n", std::numeric_limits<T>::radix);
  printf("  static constexpr int min_exponent = %d;\n", std::numeric_limits<T>::min_exponent);
  printf("  static constexpr int min_exponent10 = %d;\n", std::numeric_limits<T>::min_exponent10);
  printf("  static constexpr int max_exponent = %d;\n", std::numeric_limits<T>::max_exponent);
  printf("  static constexpr int max_exponent10 = %d;\n", std::numeric_limits<T>::max_exponent10);
  printf("  static constexpr bool traps = %s;\n", boolstr(std::numeric_limits<T>::traps));
  printf("  static constexpr bool tinyness_before = %s;\n", boolstr(std::numeric_limits<T>::tinyness_before));
  printf("\n");
  printf("  static constexpr %s min() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::min()));
  printf("  static constexpr %s lowest() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::lowest()));
  printf("  static constexpr %s max() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::max()));
  printf("  static constexpr %s epsilon() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::epsilon()));
  printf("  static constexpr %s round_error() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::round_error()));
  printf("  static constexpr %s infinity() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::infinity()));
  printf("  static constexpr %s quiet_NaN() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::quiet_NaN()));
  printf("  static constexpr %s signaling_NaN() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::signaling_NaN()));
  printf("  static constexpr %s denorm_min() noexcept { return %s; }\n", type.data(), strinify(std::numeric_limits<T>::denorm_min()));
  printf("};\n\n");
}

TEST_CASE("dummy") {
  print_for_type<bool>("bool");
  print_for_type<char>("char");
  print_for_type<signed char>("signed char");
  print_for_type<unsigned char>("unsigned char");
  //print_for_type<wchar_t>("wchar_t");
  print_for_type<short>("short");
  print_for_type<unsigned short>("unsigned short");
  print_for_type<int>("int");
  print_for_type<unsigned int>("unsigned int");
  print_for_type<long>("long");
  print_for_type<unsigned long>("unsigned long");
  print_for_type<long long>("long long");
  print_for_type<unsigned long long>("unsigned long long");
  print_for_type<float>("float");
  print_for_type<double>("double");
  print_for_type<long double>("long double");
}
