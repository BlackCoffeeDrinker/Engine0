#include "tests.hpp"

extern unsigned char testMap_160_by_50[];

using namespace e00;

namespace {
class NPCActor : public e00::Actor {

public:
  NPCActor() {
  }

  ~NPCActor() override = default;
};


e00::ResourcePtrT<e00::Map> LoadMap() {
  auto map = e00::ResourceManager::GlobalResourceManager().Make<e00::Map>("testMap_160_by_50"_id, 160, 50);

  for (uint16_t y = 0; y < 50; ++y) {
    for (uint16_t x = 0; x < 160; ++x) {
      map->Set({x, y}, testMap_160_by_50[y * 160 + x]);
    }
  }

  return map;
}
}// namespace


TEST_CASE("Can load a basic world with map", "[world]") {

}
