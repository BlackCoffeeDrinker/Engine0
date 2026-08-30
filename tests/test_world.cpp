#include "tests.hpp"

extern unsigned char testMap_160_by_50[];

using namespace e00;

namespace {
class NPCActor : public Actor {

public:
  NPCActor() {
  }

  ~NPCActor() override = default;
};


ResourcePtrT<e00::Map> LoadMap() {
  auto map = ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<e00::Map>(160, 50));

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

TEST_CASE("Query on an empty world returns nothing", "[world]") {
  World world("empty-world");

  std::vector<World::NodeID> results;
  world.Query({{0, 0}, {100, 100}}, results);

  REQUIRE(results.empty());
}

TEST_CASE("Query returns actors across multiple grid cells", "[world]") {
  World world("multi-cell-world");
  world.AddMap(ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<e00::Map>(16, 16)));

  NPCActor actorA;
  NPCActor actorB;
  NPCActor actorC;

  const auto nodeA = world.Insert(TODO, &actorA, {1, 1}); // cell (0,0)
  const auto nodeB = world.Insert(TODO, &actorB, {5, 5}); // cell (1,1)
  const auto nodeC = world.Insert(TODO, &actorC, {9, 9}); // cell (2,2)

  REQUIRE(nodeA != e00::World::InvalidNodeID);
  REQUIRE(nodeB != e00::World::InvalidNodeID);
  REQUIRE(nodeC != e00::World::InvalidNodeID);

  world.Tick(std::chrono::milliseconds(0));

  std::vector<World::NodeID> onlyFirstCell;
  world.Query({{0, 0}, {4, 4}}, onlyFirstCell);
  REQUIRE(onlyFirstCell.size() == 1);
  REQUIRE(onlyFirstCell[0] == nodeA);

  std::vector<World::NodeID> everything;
  world.Query({{0, 0}, {16, 16}}, everything);
  REQUIRE(everything.size() == 3);
  REQUIRE(std::find(everything.begin(), everything.end(), nodeA) != everything.end());
  REQUIRE(std::find(everything.begin(), everything.end(), nodeB) != everything.end());
  REQUIRE(std::find(everything.begin(), everything.end(), nodeC) != everything.end());
}

TEST_CASE("Query respects cell boundaries", "[world]") {
  World world("boundary-world");
  world.AddMap(ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<e00::Map>(16, 16)));

  NPCActor insideActor;
  NPCActor outsideActor;

  // Cell size is 4x4 tiles: (3,3) is the last tile of cell (0,0), (4,4) is the first tile of cell (1,1)
  const auto insideNode = world.Insert(TODO, &insideActor, {3, 3});
  const auto outsideNode = world.Insert(TODO, &outsideActor, {4, 4});

  REQUIRE(insideNode != e00::World::InvalidNodeID);
  REQUIRE(outsideNode != e00::World::InvalidNodeID);

  world.Tick(std::chrono::milliseconds(0));

  std::vector<World::NodeID> results;
  world.Query({{0, 0}, {4, 4}}, results);

  REQUIRE(results.size() == 1);
  REQUIRE(results[0] == insideNode);
}

TEST_CASE("Removed actor no longer appears in Query after Tick", "[world]") {
  World world("remove-world");
  world.AddMap(ResourceManager::GlobalResourceManager().TakeOwnership(std::make_unique<e00::Map>(16, 16)));

  NPCActor actor;
  const auto node = world.Insert(TODO, &actor, {2, 2});
  REQUIRE(node != e00::World::InvalidNodeID);

  world.Tick(std::chrono::milliseconds(0));

  std::vector<World::NodeID> beforeRemoval;
  world.Query({{0, 0}, {16, 16}}, beforeRemoval);
  REQUIRE(beforeRemoval.size() == 1);

  world.Remove(node);
  world.Tick(std::chrono::milliseconds(0));

  std::vector<World::NodeID> afterRemoval;
  world.Query({{0, 0}, {16, 16}}, afterRemoval);
  REQUIRE(afterRemoval.empty());
}
