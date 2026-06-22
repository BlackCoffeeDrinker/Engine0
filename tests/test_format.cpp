#include "tests.hpp"

TEST_CASE("Formatting simple characters") {
  const auto out = e00::fmt_lite::format("{}{}{}", 'a', 'b', 'c');
  REQUIRE(out == "abc");
}

TEST_CASE("Formatting with no arguments") {
  REQUIRE(e00::fmt_lite::format("test") == "test");
}

TEST_CASE("Formatting arguments with position") {
  const auto out = e00::fmt_lite::format("{1}{0}{2}", 'b', 'a', 'c');
  REQUIRE(out == "abc");
}

TEST_CASE("Formatting arguments with position and arguments") {
  const auto out = e00::fmt_lite::format("{1:SOMETHING}{0}{2}", 'b', 'a', 'c');
  REQUIRE(out == "abc");
}

TEST_CASE("Formatting arguments with arguments") {
  const auto out = e00::fmt_lite::format("{:SOMETHING}{}{}", 'a', 'b', 'c');
  REQUIRE(out == "abc");
}

TEST_CASE("Formatting arguments with mixed position") {
  const auto out = e00::fmt_lite::format("{2}{1}{}", 'c', 'b', 'a');
  REQUIRE(out == "abc");
}

TEST_CASE("Formatting integers") {
  REQUIRE(e00::fmt_lite::format("{}", 0) == "0");
  REQUIRE(e00::fmt_lite::format("{}", 1) == "1");
  REQUIRE(e00::fmt_lite::format("{}", 10) == "10");
  REQUIRE(e00::fmt_lite::format("{}", 100) == "100");
  REQUIRE(e00::fmt_lite::format("{}", -1) == "-1");
  REQUIRE(e00::fmt_lite::format("{}", -10) == "-10");
  REQUIRE(e00::fmt_lite::format("{}", std::numeric_limits<int>::min()) == "-2147483648");
}

TEST_CASE("Formatting unsigned integers") {
  REQUIRE(e00::fmt_lite::format("{}", 0u) == "0");
  REQUIRE(e00::fmt_lite::format("{}", 1u) == "1");
  REQUIRE(e00::fmt_lite::format("{}", 10u) == "10");
}

TEST_CASE("Formatting shorts") {
  short s0 = 0;
  short s1 = 1;
  REQUIRE(e00::fmt_lite::format("{}", s0) == "0");
  REQUIRE(e00::fmt_lite::format("{}", s1) == "1");
}

TEST_CASE("Formatting floats") {
  REQUIRE(e00::fmt_lite::format("{}", 0.0f) == "0");
  REQUIRE(e00::fmt_lite::format("{}", 1.25f) == "1.25");
}
