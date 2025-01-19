#include <Engine.hpp>
#include <catch2/catch.hpp>

extern unsigned char testMap_160_by_50[];

namespace {
auto BuildWorld() {
  auto map = e00::ResourceManager::GlobalResourceManager().Make<e00::Map>("testMap_160_by_50", 160, 50);

  for (auto x = 0; x < map->Width(); x++) {
    for (auto y = 0; y < map->Height(); y++) {
      map->Set(e00::Map::Position(x, y), testMap_160_by_50[y * map->Width() + x]);
    }
  }

  return std::make_unique<e00::World>("test world", std::move(map));
}
}// namespace

TEST_CASE("Widget test") {
  e00::Bitmap target({800,600}, e00::Bitmap::BitDepth::DEPTH_32);
  
  e00::Painter painter(target);
  auto aWorld = BuildWorld();

  e00::Widget w;
  e00::WorldWidget ww(aWorld);

  ww.SetParent(&w);
  w.SetFixedSize({640, 480});

  REQUIRE(w.Size().x == 640);
  REQUIRE(w.Size().y == 480);

  w.Paint(painter);
  
}
