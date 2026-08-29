#include <catch2/catch_test_macros.hpp>

#include <Engine/Detail/StringFormat.hpp>
#include <Engine/Detail/TypeId.hpp>
#include <Engine/Logging/Logger.hpp>
#include <span>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/World.hpp>

#include "Actors/ActorMobile.hpp"

using namespace e00;

namespace {
// A single-frame sprite whose only pixel is `markerColor`, so painting it can be verified
// by reading back that pixel's raw index.
ResourcePtrT<Sprite> MakeTestSprite(uint8_t markerColor) {
  auto sprite = Sprite::Create(Vec2D<BitmapSizeType>{1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  auto frame = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  {
    auto painter = frame->BeginDraw();
    painter->SetPenSolid(1, markerColor);
    painter->DrawPoint({0, 0});
  }
  REQUIRE_FALSE(sprite->AddFrame(std::move(frame), std::chrono::milliseconds(100)));
  return ResourceManager::GlobalResourceManager().TakeOwnership(std::move(sprite));
}

// A two-frame sprite (marker `firstColor` then `secondColor`) used to prove animation only
// advances while the actor is actively moving.
ResourcePtrT<Sprite> MakeTwoFrameSprite(uint8_t firstColor, uint8_t secondColor) {
  auto sprite = Sprite::Create(Vec2D<BitmapSizeType>{1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  for (const auto color: {firstColor, secondColor}) {
    auto frame = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
    auto painter = frame->BeginDraw();
    painter->SetPenSolid(1, color);
    painter->DrawPoint({0, 0});
    REQUIRE_FALSE(sprite->AddFrame(std::move(frame), std::chrono::milliseconds(100)));
  }
  return ResourceManager::GlobalResourceManager().TakeOwnership(std::move(sprite));
}

uint8_t ReadMarkerPixel(const Bitmap &bmp) {
  std::vector<uint8_t> buf(1);
  const DrawableSurface::TargetInformation info{DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, nullptr, {}, {}};
  bmp.ReadLineInto(0, 0, 1, info, buf);
  return buf[0];
}
}// namespace

TEST_CASE("ActorMobile with no patrol points stays idle", "[actor][mobile]") {
  World world("idle-world", {1000, 1000});
  impl::ActorMobile actor;
  actor.SetSpeed(10);

  const auto node = world.Insert("npc", &actor, {5, 5});
  REQUIRE(node != World::InvalidNodeID);

  world.Tick(std::chrono::milliseconds(1000));

  REQUIRE(actor.CurrentPatrolState() == impl::ActorMobile::PatrolState::Idle);
  REQUIRE(world.PositionOf(node) == WorldPosition{5, 5});
}

TEST_CASE("ActorMobile patrols between waypoints and cycles back to the first one", "[actor][mobile]") {
  World world("patrol-world", {1000, 1000});
  impl::ActorMobile actor;
  actor.SetSpeed(10);// 10 world units / second

  REQUIRE(actor.AddPatrolPoint({0, 0}));
  REQUIRE(actor.AddPatrolPoint({10, 0}));
  REQUIRE(actor.AddPatrolPoint({10, 10}));
  REQUIRE(actor.PatrolPointCount() == 3);

  const auto node = world.Insert("npc", &actor, {0, 0});
  REQUIRE(node != World::InvalidNodeID);

  // First waypoint is the starting position: arrives immediately and advances to the next one.
  world.Tick(std::chrono::milliseconds(1000));
  REQUIRE(world.PositionOf(node) == WorldPosition{0, 0});
  REQUIRE(actor.CurrentPatrolState() == impl::ActorMobile::PatrolState::Waiting);

  // Covers exactly one tick's worth of distance (10 units at 10 units/sec): arrives.
  world.Tick(std::chrono::milliseconds(1000));
  REQUIRE(world.PositionOf(node) == WorldPosition{10, 0});

  world.Tick(std::chrono::milliseconds(1000));
  REQUIRE(world.PositionOf(node) == WorldPosition{10, 10});

  // Cycled back to the first waypoint (0,0), which is farther than one tick's step (diagonal distance ~14.1 > 10):
  // the actor should be moving, not yet arrived.
  world.Tick(std::chrono::milliseconds(1000));
  const auto afterFourthTick = world.PositionOf(node);
  REQUIRE(afterFourthTick != WorldPosition{10, 10});
  REQUIRE(afterFourthTick != WorldPosition{0, 0});
  REQUIRE(actor.CurrentPatrolState() == impl::ActorMobile::PatrolState::Moving);
}

TEST_CASE("ActorMobile patrol array rejects points beyond capacity", "[actor][mobile]") {
  impl::ActorMobile actor;

  for (size_t i = 0; i < impl::kMaxPatrolPoints; i++) {
    REQUIRE(actor.AddPatrolPoint({static_cast<WorldCoordinateType>(i), 0}));
  }

  REQUIRE(actor.PatrolPointCount() == impl::kMaxPatrolPoints);
  REQUIRE_FALSE(actor.AddPatrolPoint({100, 100}));
  REQUIRE(actor.PatrolPointCount() == impl::kMaxPatrolPoints);
}

TEST_CASE("ActorMobile::Tick updates the element's facing to match the direction of travel", "[actor][mobile][facing]") {
  World world("facing-patrol-world", {1000, 1000});
  impl::ActorMobile actor;
  actor.SetSpeed(10);
  REQUIRE(actor.AddPatrolPoint({100, 0}));// straight east

  const auto node = world.Insert("npc", &actor, {0, 0});
  REQUIRE(node != World::InvalidNodeID);

  world.Tick(std::chrono::milliseconds(1000));
  REQUIRE(world.FacingOf(node) == Direction::East);
}

TEST_CASE("ActorMobile::Draw selects the sprite matching the collapsed facing direction", "[actor][mobile][draw]") {
  impl::ActorMobile actor;
  actor.SetDirectionSprite(Direction::North, MakeTestSprite(10));
  actor.SetDirectionSprite(Direction::East, MakeTestSprite(20));
  actor.SetDirectionSprite(Direction::South, MakeTestSprite(30));
  actor.SetDirectionSprite(Direction::West, MakeTestSprite(40));

  auto canvas = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);

  {
    auto painter = canvas->BeginDraw();
    actor.Draw(*painter, {0, 0}, Direction::East, std::chrono::milliseconds(0));
  }
  REQUIRE(ReadMarkerPixel(*canvas) == 20);

  {
    auto painter = canvas->BeginDraw();
    // Diagonal facing collapses to the nearest cardinal (NorthEast -> North).
    actor.Draw(*painter, {0, 0}, Direction::NorthEast, std::chrono::milliseconds(0));
  }
  REQUIRE(ReadMarkerPixel(*canvas) == 10);
}

TEST_CASE("ActorMobile::Draw with an unset direction sprite is a graceful no-op", "[actor][mobile][draw]") {
  impl::ActorMobile actor;
  actor.SetDirectionSprite(Direction::East, MakeTestSprite(20));
  // North is deliberately left unset.

  auto canvas = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  auto painter = canvas->BeginDraw();
  REQUIRE_NOTHROW(actor.Draw(*painter, {0, 0}, Direction::North, std::chrono::milliseconds(0)));
}

TEST_CASE("ActorMobile::Draw only animates the walk cycle while PatrolState is Moving", "[actor][mobile][draw]") {
  World world("draw-anim-world", {1000, 1000});
  impl::ActorMobile actor;
  actor.SetSpeed(100);
  actor.SetDirectionSprite(Direction::East, MakeTwoFrameSprite(1, 2));

  auto canvas = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);

  SECTION("Idle: holds frame 0 regardless of animation time") {
    const auto node = world.Insert("npc", &actor, {0, 0});
    REQUIRE(node != World::InvalidNodeID);
    world.Tick(std::chrono::milliseconds(16));
    REQUIRE(actor.CurrentPatrolState() == impl::ActorMobile::PatrolState::Idle);

    auto painter = canvas->BeginDraw();
    actor.Draw(*painter, {0, 0}, Direction::East, std::chrono::milliseconds(150));
    REQUIRE(ReadMarkerPixel(*canvas) == 1);
  }

  SECTION("Moving: advances the frame based on animation time") {
    REQUIRE(actor.AddPatrolPoint({500, 0}));// far enough that one tick doesn't arrive
    const auto node = world.Insert("npc", &actor, {0, 0});
    REQUIRE(node != World::InvalidNodeID);
    world.Tick(std::chrono::milliseconds(100));
    REQUIRE(actor.CurrentPatrolState() == impl::ActorMobile::PatrolState::Moving);

    auto painter = canvas->BeginDraw();
    // Each frame lasts 100ms; 150ms lands in the second frame.
    actor.Draw(*painter, {0, 0}, Direction::East, std::chrono::milliseconds(150));
    REQUIRE(ReadMarkerPixel(*canvas) == 2);
  }
}
