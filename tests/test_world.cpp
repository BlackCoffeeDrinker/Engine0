
#include <catch2/catch.hpp>
#include <Engine.hpp>

extern unsigned char testMap_160_by_50[];
/*

namespace {
e00::World BuildWorld() {
  e00::Map map(160, 50);

  auto code = map.LoadBulk(testMap_160_by_50);
  REQUIRE(code.value() == 0);

  e00::Tileset tileset(map.HighestTitleId() + 1);

  return {std::move(tileset), std::move(map)};
}
}

TEST_CASE("In a world") {
  class SimpleTileChar {
    char _t;
  public:
    explicit SimpleTileChar(char t) : _t(t) {}
    ~SimpleTileChar() = default;

    [[nodiscard]] char Get() const { return _t; }
  };

  e00::World world = BuildWorld();

  {
    world.CreateComponent<SimpleTileChar>(0, '+');
    world.CreateComponent<SimpleTileChar>(1, 'A');
    world.CreateComponent<SimpleTileChar>(2, 'B');
    world.CreateComponent<SimpleTileChar>(3, 'C');
    world.CreateComponent<SimpleTileChar>(4, 'D');
    world.CreateComponent<SimpleTileChar>(5, 'E');
    world.CreateComponent<SimpleTileChar>(6, 'F');
    world.CreateComponent<SimpleTileChar>(7, '.');
    world.CreateComponent<SimpleTileChar>(8, 'H');
    world.CreateComponent<SimpleTileChar>(9, 'I');
    world.CreateComponent<SimpleTileChar>(10, 'J');
    world.CreateComponent<SimpleTileChar>(11, 'K');
    world.CreateComponent<SimpleTileChar>(12, 'L');
    world.CreateComponent<SimpleTileChar>(13, 'M');
    world.CreateComponent<SimpleTileChar>(14, 'N');
    world.CreateComponent<SimpleTileChar>(15, 'O');
    world.CreateComponent<SimpleTileChar>(24, 'P');
    world.CreateComponent<SimpleTileChar>(25, 'Q');
    world.CreateComponent<SimpleTileChar>(27, 'R');
    world.CreateComponent<SimpleTileChar>(30, 'S');
    world.CreateComponent<SimpleTileChar>(46, 'T');
    world.CreateComponent<SimpleTileChar>(62, 'U');
    world.CreateComponent<SimpleTileChar>(76, 'V');
    world.CreateComponent<SimpleTileChar>(78, 'W');
    world.CreateComponent<SimpleTileChar>(93, 'X');
    world.CreateComponent<SimpleTileChar>(116, 'Y');
    world.CreateComponent<SimpleTileChar>(132, 'Z');
    world.CreateComponent<SimpleTileChar>(135, '!');
    world.CreateComponent<SimpleTileChar>(138, '@');
    world.CreateComponent<SimpleTileChar>(140, '#');
    world.CreateComponent<SimpleTileChar>(142, ':');
    world.CreateComponent<SimpleTileChar>(145, '$');
    world.CreateComponent<SimpleTileChar>(155, '%');
    world.CreateComponent<SimpleTileChar>(213, '*');
  }

  printf("W: %d\nH: %d\n", world.Width(), world.Height());
  for (uint16_t y = 0; y < world.Height(); y++) {
    for (uint16_t x = 0; x < world.Width(); x++) {
      (void) putc(world.GetComponent<SimpleTileChar>({x, y})->Get(), stdout);
    }
    (void) putc('\n', stdout);
  }

  printf("\n\n-----------------------------------\n\n\n");


  const auto window = world.MakeWindow({160, 50}, {80, 25});
  for (e00::WorldCoordinateType y = 0; y < window.Height(); y++) {
    for (e00::WorldCoordinateType x = 0; x < window.Width(); x++) {
      (void) putc(window.GetComponent<SimpleTileChar>({x, y})->Get(), stdout);
    }
    (void) putc('\n', stdout);
  }
}

TEST_CASE("Some test case") {
  auto world = BuildWorld();
  e00::Actor a;

  world.Insert(&a, {10, 10});

  const auto window = world.MakeWindow({0, 0}, {80, 25});



}
*/
