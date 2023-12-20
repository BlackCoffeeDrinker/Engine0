#include <catch2/catch.hpp>
#include <Engine.hpp>

extern unsigned char testMap_160_by_50[];
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
