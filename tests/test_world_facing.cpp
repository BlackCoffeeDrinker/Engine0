#include <catch2/catch_test_macros.hpp>

#include <Engine/Detail/StringFormat.hpp>
#include <Engine/Detail/TypeId.hpp>
#include <Engine/Direction.hpp>
#include <Engine/Logging/Logger.hpp>
#include <span>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Resource/Tileset.hpp>
#include <Engine/World.hpp>

using namespace e00;

namespace {
// A minimal Actor used to prove World::Paint feeds Draw with the element's real facing
// and the world's shared elapsed-time clock, without needing a concrete sprite-drawing actor.
class RecordingActor : public Actor {
public:
  mutable bool drawCalled{false};
  mutable Direction lastFacing{Direction::North};
  mutable std::chrono::milliseconds lastAnimTime{0};

  RecordingActor() { SetType(BodyType::Static); }

  [[nodiscard]] type_t Type() const override { return type_id<RecordingActor>(); }

  void Draw(Painter &painter, const BitmapPosition &position, Direction facing, std::chrono::milliseconds animTime) const override {
    (void) painter;
    (void) position;
    drawCalled = true;
    lastFacing = facing;
    lastAnimTime = animTime;
  }

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override {
    static const State kEmptyState{0, {}};
    return kEmptyState;
  }
};
}// namespace

TEST_CASE("World::SetFacing/FacingOf round-trip for a Static actor", "[world][facing]") {
  World world("facing-world", {100, 100});
  RecordingActor actor;

  const auto node = world.Insert("static-actor", &actor, {5, 5});
  REQUIRE(node != World::InvalidNodeID);

  // Default facing is South.
  REQUIRE(world.FacingOf(node) == Direction::South);

  world.SetFacing(node, Direction::NorthEast);
  REQUIRE(world.FacingOf(node) == Direction::NorthEast);
}

TEST_CASE("World::FacingOf/SetFacing behave safely for an out-of-range NodeID", "[world][facing]") {
  World world("facing-world", {100, 100});

  REQUIRE(world.FacingOf(World::InvalidNodeID) == Direction::South);
  // Must not crash.
  world.SetFacing(World::InvalidNodeID, Direction::North);
}

TEST_CASE("World::ElapsedTime accumulates the sum of Tick deltas", "[world][clock]") {
  World world("clock-world", {100, 100});
  REQUIRE(world.ElapsedTime() == std::chrono::milliseconds(0));

  world.Tick(std::chrono::milliseconds(16));
  world.Tick(std::chrono::milliseconds(20));
  world.Tick(std::chrono::milliseconds(4));

  REQUIRE(world.ElapsedTime() == std::chrono::milliseconds(40));
}

TEST_CASE("World::Paint passes the element's stored facing and ElapsedTime into Draw", "[world][paint]") {
  auto tilesetResource = std::make_unique<Tileset>();
  tilesetResource->SetTileSize({1, 1});
  auto tileset = ResourceManager::GlobalResourceManager().TakeOwnership(std::move(tilesetResource));

  World world("paint-world", {20, 20});
  REQUIRE(world.AddTileset(1, tileset));

  RecordingActor actor;
  const auto node = world.Insert("recorder", &actor, {5, 5});
  REQUIRE(node != World::InvalidNodeID);

  world.SetFacing(node, Direction::West);
  world.Tick(std::chrono::milliseconds(250));// populates the grid and advances the clock

  auto canvas = Bitmap::Create({20, 20}, DrawableSurface::BitDepth::DEPTH_8, 2);
  auto painter = canvas->BeginDraw();
  world.Paint(*painter, {0, 0}, {20, 20}, {0, 0});

  REQUIRE(actor.drawCalled);
  REQUIRE(actor.lastFacing == Direction::West);
  REQUIRE(actor.lastAnimTime == world.ElapsedTime());
}

TEST_CASE("CollapseTo4Way maps all 8 directions to one of the 4 cardinals", "[direction]") {
  CHECK(CollapseTo4Way(Direction::North) == Direction::North);
  CHECK(CollapseTo4Way(Direction::NorthEast) == Direction::North);
  CHECK(CollapseTo4Way(Direction::East) == Direction::East);
  CHECK(CollapseTo4Way(Direction::SouthEast) == Direction::South);
  CHECK(CollapseTo4Way(Direction::South) == Direction::South);
  CHECK(CollapseTo4Way(Direction::SouthWest) == Direction::South);
  CHECK(CollapseTo4Way(Direction::West) == Direction::West);
  CHECK(CollapseTo4Way(Direction::NorthWest) == Direction::North);
}
