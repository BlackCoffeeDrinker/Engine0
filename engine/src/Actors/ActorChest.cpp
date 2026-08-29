
#include "ActorChest.hpp"

#include <Engine/Platform/Stream.hpp>

namespace e00::impl {

ActorChest::ActorChest() {
  SetType(BodyType::Static);
  // Default sprites are optional: unit tests construct chests without a bootstrapped
  // ResourceManager, and game content sets CHEST_CLOSED/OPENED via properties or explicit calls.
}

bool ActorChest::OnInteract(World &world, ActorId self) {
  (void) world;
  (void) self;
  _open = !_open;
  return true;
}

void ActorChest::SetClosedSprite(ResourcePtrT<Sprite> sprite) {
  for (const auto direction: {Direction::North, Direction::East, Direction::South, Direction::West}) {
    SetStateSprite(kClosedStateId, direction, sprite);
  }
}

void ActorChest::SetOpenSprite(ResourcePtrT<Sprite> sprite) {
  for (const auto direction: {Direction::North, Direction::East, Direction::South, Direction::West}) {
    SetStateSprite(kOpenStateId, direction, sprite);
  }
}

bool ActorChest::SaveState(WritableStream &out) const {
  const uint8_t openFlag = _open ? 1 : 0;
  return !out.Write(sizeof(openFlag), &openFlag);
}

bool ActorChest::LoadState(Stream &in) {
  uint8_t openFlag = 0;
  if (in.Read(sizeof(openFlag), &openFlag)) {
    return false;
  }

  _open = openFlag != 0;
  return true;
}

}// namespace e00::impl
