#include <catch2/catch.hpp>
#include <Engine.hpp>

TEST_CASE("Source location shows actual source") {
  e00::source_location r = e00::source_location::current();

  std::string r_filename = r.file_name();
  REQUIRE(r_filename == __FILE__);
  REQUIRE(r.line() == 5);
}
