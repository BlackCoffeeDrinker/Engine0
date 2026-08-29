#include "ActorWarpDoor.hpp"

namespace e00::impl {

bool ActorWarpDoor::OnInteract(World &world, ActorId self) {
  const auto handled = ActorDoor::OnInteract(world, self);

  if (IsOpen()) {
    _activated = true;

    // TODO: Tell engine to load the target world
  }

  return handled;
}

}// namespace e00::impl
