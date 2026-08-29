#include <catch2/catch_test_macros.hpp>

#include <Engine/Platform/Stream.hpp>
#include <Engine/World.hpp>

#include "Actors/ActorDecoration.hpp"
#include "Actors/ActorDoor.hpp"
#include "Actors/ActorWarpDoor.hpp"

#include <cstring>

using namespace e00;

namespace {
class MemoryStream : public Stream {
  std::vector<uint8_t> _data;

protected:
  std::error_code real_read(size_t size, void *data) override {
    std::memcpy(data, _data.data() + Position(), size);
    return {};
  }

  std::error_code real_seek(size_t) override { return {}; }

public:
  explicit MemoryStream(std::vector<uint8_t> data) : Stream(data.size()), _data(std::move(data)) {}
};

class MemoryWritableStream : public WritableStream {
public:
  std::vector<uint8_t> data;

  MemoryWritableStream() : WritableStream(0) {}

protected:
  std::error_code real_write(size_t size, const void *ptr) override {
    const auto *bytes = static_cast<const uint8_t *>(ptr);
    data.insert(data.end(), bytes, bytes + size);
    return {};
  }

  std::error_code real_read(size_t, void *) override { return std::make_error_code(std::errc::function_not_supported); }
  std::error_code real_seek(size_t) override { return {}; }
};
}// namespace

TEST_CASE("ActorDoor starts closed and blocking", "[actor][door]") {
  impl::ActorDoor door;
  REQUIRE_FALSE(door.IsOpen());
  REQUIRE(door.PhysicsType() == Actor::BodyType::Static);
  REQUIRE(door.CurrentState().stateId == impl::ActorDoor::kClosedStateId);
}

TEST_CASE("ActorDoor::OnInteract toggles open/closed and stops blocking once open", "[actor][door]") {
  World world("door-world", {100, 100});
  impl::ActorDoor door;
  const auto node = world.Insert("door", &door, {1, 1});
  REQUIRE(node != World::InvalidNodeID);

  REQUIRE(world.Interact(node));
  REQUIRE(door.IsOpen());
  REQUIRE(door.PhysicsType() == Actor::BodyType::None);
  REQUIRE(door.CurrentState().stateId == impl::ActorDoor::kOpenStateId);

  REQUIRE(world.Interact(node));
  REQUIRE_FALSE(door.IsOpen());
  REQUIRE(door.PhysicsType() == Actor::BodyType::Static);
  REQUIRE(door.CurrentState().stateId == impl::ActorDoor::kClosedStateId);
}

TEST_CASE("ActorDoor save/load round trip preserves open state and blocking", "[actor][door]") {
  impl::ActorDoor openDoor;
  REQUIRE(openDoor.HasSavableState());

  World world("door-world", {100, 100});
  const auto node = world.Insert("door", &openDoor, {1, 1});
  REQUIRE(world.Interact(node));
  REQUIRE(openDoor.IsOpen());

  MemoryWritableStream out;
  REQUIRE(openDoor.SaveState(out));

  impl::ActorDoor reloadedDoor;
  REQUIRE_FALSE(reloadedDoor.IsOpen());

  MemoryStream in(out.data);
  REQUIRE(reloadedDoor.LoadState(in));
  REQUIRE(reloadedDoor.IsOpen());
  REQUIRE(reloadedDoor.PhysicsType() == Actor::BodyType::None);
}

TEST_CASE("ActorWarpDoor behaves like a door and additionally exposes a pending warp request once opened", "[actor][door][warp]") {
  World world("warp-door-world", {100, 100});
  impl::ActorWarpDoor warpDoor;
  warpDoor.SetTargetWorld("other_world");
  warpDoor.SetTargetEntry("entrance");

  REQUIRE_FALSE(warpDoor.IsActivated());

  const auto node = world.Insert("warp_door", &warpDoor, {1, 1});
  REQUIRE(node != World::InvalidNodeID);

  REQUIRE(world.Interact(node));
  REQUIRE(warpDoor.IsOpen());
  REQUIRE(warpDoor.PhysicsType() == Actor::BodyType::None);
  REQUIRE(warpDoor.IsActivated());
  REQUIRE(warpDoor.TargetWorld() == "other_world");
  REQUIRE(warpDoor.TargetEntry() == "entrance");

  warpDoor.ClearActivation();
  REQUIRE_FALSE(warpDoor.IsActivated());

  // Closing it back doesn't re-activate it.
  REQUIRE(world.Interact(node));
  REQUIRE_FALSE(warpDoor.IsOpen());
  REQUIRE_FALSE(warpDoor.IsActivated());
}

TEST_CASE("ActorDecoration is non-colliding by default and can be made collidable", "[actor][decoration]") {
  impl::ActorDecoration decoration;
  REQUIRE_FALSE(decoration.IsCollidable());
  REQUIRE(decoration.PhysicsType() == Actor::BodyType::None);

  decoration.SetCollidable(true);
  REQUIRE(decoration.IsCollidable());
  REQUIRE(decoration.PhysicsType() == Actor::BodyType::Static);

  decoration.SetCollidable(false);
  REQUIRE_FALSE(decoration.IsCollidable());
  REQUIRE(decoration.PhysicsType() == Actor::BodyType::None);
}

TEST_CASE("ActorDecoration has no savable state", "[actor][decoration]") {
  impl::ActorDecoration decoration;
  REQUIRE_FALSE(decoration.HasSavableState());
}
