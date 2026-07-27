#include "tests.hpp"

using namespace e00;

extern unsigned char testMap_160_by_50[];

TEST_CASE("PaintMap does not crash when tile size was never configured", "[map]") {
  e00::ResourceManager::GlobalResourceManager().SetAlias("labeled_overworldtiles.png"_id, "tests/labeled_overworldtiles.png");

  auto map = e00::ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<e00::Map>(4, 4));
  map->SetLayerCount(1);
  for (e00::WorldCoordinateType y = 0; y < map->Height(); y++) {
    for (e00::WorldCoordinateType x = 0; x < map->Width(); x++) {
      REQUIRE(map->Set(0, e00::Position(x, y), 1));
    }
  }

  map->SetTileset(e00::ResourceManager::GlobalResourceManager().LazyResource<e00::Bitmap>("labeled_overworldtiles.png"_id));
  // Note: map->SetTileSize(...) is intentionally NOT called here.

  auto target = e00::Bitmap::Create({64, 64}, e00::DrawableSurface::BitDepth::DEPTH_32);
  auto painter = target->BeginDraw();

  REQUIRE_NOTHROW(map->PaintMap({{0, 0}, {4, 4}}, *painter, {0, 0}));
}

/*

TEST_CASE("Map can have data") {
  e00::Map map(1, 1);

  REQUIRE(map.Set({0, 0}, 1));
  REQUIRE(map.Get({0, 0}) == 1);
}

TEST_CASE("World building") {
  e00::Map map(2, 2);
  REQUIRE(map.Set({0, 0}, 0));
  REQUIRE(map.Set({1, 0}, 1));
  REQUIRE(map.Set({0, 1}, 2));
  REQUIRE(map.Set({1, 1}, 3));

  e00::Tileset t(5);

  e00::World world(std::move(t), std::move(map));

  class SimpleComponent {
  public:
    int a{};
  };

  {
    auto sc = world.CreateComponentAt<SimpleComponent>({0, 0});
    sc->a = 1;
  }

  REQUIRE(world.GetComponent<SimpleComponent>({0, 0}) != nullptr);
  REQUIRE(world.GetComponent<SimpleComponent>({0, 0})->a == 1);
}

TEST_CASE("Can load a map") {
  e00::Map map(160, 50);

  auto code = map.LoadBulk(testMap_160_by_50);
  REQUIRE(code.value() == 0);
  const auto lastId = map.Get({159, 49});
  REQUIRE(lastId == 0x07);
  const auto firstId = map.Get({0, 0});
  REQUIRE(firstId == 0x87);
}

*/
