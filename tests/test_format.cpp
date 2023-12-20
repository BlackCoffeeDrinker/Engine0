#include <catch2/catch.hpp>
#include <Engine.hpp>


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
