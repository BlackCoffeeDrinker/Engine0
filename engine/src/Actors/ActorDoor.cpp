#include "ActorDoor.hpp"

#include <Engine/Platform/Stream.hpp>

namespace e00::impl {

ActorDoor::ActorDoor() {
  UpdateBodyType();
}

bool ActorDoor::OnInteract(World &world, ActorId self) {
  (void) world;
  (void) self;
  _open = !_open;
  UpdateBodyType();
  return true;
}

void ActorDoor::SetClosedSprite(ResourcePtrT<Sprite> sprite) {
  for (auto direction: {Direction::North, Direction::East, Direction::South, Direction::West}) {
    SetStateSprite(kClosedStateId, direction, sprite);
  }
}

void ActorDoor::SetOpenSprite(ResourcePtrT<Sprite> sprite) {
  for (auto direction: {Direction::North, Direction::East, Direction::South, Direction::West}) {
    SetStateSprite(kOpenStateId, direction, sprite);
  }
}

bool ActorDoor::SaveState(WritableStream &out) const {
  const uint8_t openFlag = _open ? 1 : 0;
  return !out.Write(sizeof(openFlag), &openFlag);
}

bool ActorDoor::LoadState(Stream &in) {
  uint8_t openFlag = 0;
  if (in.Read(sizeof(openFlag), &openFlag)) {
    return false;
  }

  _open = openFlag != 0;
  UpdateBodyType();
  return true;
}

}// namespace e00::impl
