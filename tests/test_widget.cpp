#include "tests.hpp"

extern unsigned char testMap_160_by_50[];

using namespace e00;

namespace {
auto BuildWorld() {
  ResourceManager::GlobalResourceManager().SetAlias("labeled_overworldtiles.png"_id, "tests/labeled_overworldtiles.png");
  auto map = ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<Map>(160, 50));

  for (auto x = 0; x < map->Width(); x++) {
    for (auto y = 0; y < map->Height(); y++) {
      map->Set(TilePosition(x, y), testMap_160_by_50[y * map->Width() + x]);
    }
  }

  map->SetTileset(ResourceManager::GlobalResourceManager().LazyResource<Bitmap>("labeled_overworldtiles.png"_id));
  map->SetTilesetSpacing(1);
  map->SetTileSize({16, 16});

  auto world = std::make_unique<World>("test world");
  world->AddMap(std::move(map));

  // Add some actors


  return world;
}
}// namespace

TEST_CASE("Widget test", "Widgets") {
  auto target = Bitmap::Create({800, 600}, DrawableSurface::BitDepth::DEPTH_32);

  auto painter = target->BeginDraw();
  auto aWorld = BuildWorld();

  WorldWidget ww(aWorld);
  ww.Resize({800, 600});

  ww.Paint(*painter);

  auto wstream = StreamFactory::GlobalStreamFactory().OpenStreamForWrite("test.bmp");
  REQUIRE(wstream != nullptr);
  target->SaveToBMP(*wstream);
}
