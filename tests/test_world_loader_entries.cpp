#include <catch2/catch_test_macros.hpp>

#include <span>
#include <cstring>
#include <string>
#include <vector>

#include <Engine.hpp>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Platform/Stream.hpp>

#include "WorldLoader.hpp"

using namespace e00;

namespace {
class MemoryStream : public Stream {
  std::vector<uint8_t> _data;

protected:
  std::error_code real_read(size_t size, void *data) override {
    std::memcpy(data, _data.data() + Position(), size);
    return {};
  }

  std::error_code real_seek(size_t) override { return {}; }

public:
  explicit MemoryStream(std::string_view text)
      : Stream(text.size()), _data(text.begin(), text.end()) {}
};
}// namespace

TEST_CASE("WorldLoader parses named [entry:name] sections", "[world][loader][entry]") {
  constexpr std::string_view kIni =
      "[map]\n"
      "width = 4\n"
      "height = 4\n"
      "tileset = 1:dummy_tileset\n"
      "\n"
      "[set]\n"
      "data = 1,1,1,1,\n"
      "data = 1,1,1,1,\n"
      "data = 1,1,1,1,\n"
      "data = 1,1,1,1,\n"
      "\n"
      "[entry:default]\n"
      "position = 16, 32\n"
      "\n"
      "[entry:from_mines]\n"
      "position = 100, 200\n"
      "\n"
      "[object:DOOR1]\n"
      "source = door\n"
      "position = 8, 8\n";

  MemoryStream stream(kIni);
  impl::WorldLoader loader(&ResourceManager::GlobalResourceManager());
  const auto ctx = loader.Load(stream);

  REQUIRE(ctx.width == 4);
  REQUIRE(ctx.height == 4);
  REQUIRE(ctx.entries.size() == 2);
  REQUIRE(ctx.entries.contains("default"));
  REQUIRE(ctx.entries.at("default") == WorldPosition{16, 32});
  REQUIRE(ctx.entries.contains("from_mines"));
  REQUIRE(ctx.entries.at("from_mines") == WorldPosition{100, 200});
  REQUIRE(ctx.actors.contains("DOOR1"));
  REQUIRE(ctx.actors.at("DOOR1").position == WorldPosition{8, 8});
}
