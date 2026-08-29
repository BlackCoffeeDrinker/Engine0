#include "ActorDecoration.hpp"

namespace e00::impl {

ActorDecoration::ActorDecoration() {
  SetType(BodyType::None);
}

void ActorDecoration::SetSprite(ResourcePtrT<Sprite> sprite) {
  for (auto direction: {Direction::North, Direction::East, Direction::South, Direction::West}) {
    SetStateSprite(kStateId, direction, sprite);
  }
}

}// namespace e00::impl
