#include "tests.hpp"

extern unsigned char testMap_160_by_50[];

using namespace e00;

namespace {
auto BuildWorld() {
  e00::ResourceManager::GlobalResourceManager().SetAlias("labeled_overworldtiles.png"_id, "tests/labeled_overworldtiles.png");
  auto map = e00::ResourceManager::GlobalResourceManager().Make<e00::Map>("testMap_160_by_50"_id, 160, 50);

  for (auto x = 0; x < map->Width(); x++) {
    for (auto y = 0; y < map->Height(); y++) {
      map->Set(e00::Position(x, y), testMap_160_by_50[y * map->Width() + x]);
    }
  }

  map->SetTileset(e00::ResourceManager::GlobalResourceManager().LazyResource<e00::Bitmap>("labeled_overworldtiles.png"_id));
  map->SetTilesetSpacing(1);
  map->SetTileSize({16, 16});

  auto world = std::make_unique<e00::World>("test world");
  world->AddMap(std::move(map));

  // Add some actors


  return world;
}
}// namespace

TEST_CASE("Widget test", "Widgets") {
  auto target = e00::Bitmap::Create({800, 600}, e00::DrawableSurface::BitDepth::DEPTH_32);

  auto painter = target->BeginDraw();
  auto aWorld = BuildWorld();

  e00::WorldWidget ww(aWorld);
  ww.Resize({800, 600});

  ww.Paint(*painter);

  auto wstream = e00::StreamFactory::GlobalStreamFactory().OpenStreamForWrite("test.bmp");
  REQUIRE(wstream != nullptr);
  target->SaveToBMP(*wstream);
}
