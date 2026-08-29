#include "ActorMobile.hpp"

#include <cmath>

namespace e00::impl {
namespace {
Direction DirectionFromVector(double dx, double dy) {
  // Screen/world space: +y is down. Map the angle to the closest of 8 directions.
  const auto angle = std::atan2(dy, dx);
  constexpr auto pi = 3.14159265358979323846;
  constexpr auto slice = pi / 4.0;
  const auto normalized = std::fmod(angle + (2.0 * pi), 2.0 * pi);
  const auto index = static_cast<int>(std::lround(normalized / slice)) % 8;

  switch (index) {
    case 0: return Direction::East;
    case 1: return Direction::SouthEast;
    case 2: return Direction::South;
    case 3: return Direction::SouthWest;
    case 4: return Direction::West;
    case 5: return Direction::NorthWest;
    case 6: return Direction::North;
    case 7: return Direction::NorthEast;
    default: return Direction::South;
  }
}
}// namespace

ActorMobile::ActorMobile() { SetType(BodyType::Dynamic); }

bool ActorMobile::AddPatrolPoint(const WorldPosition &point) {
  if (_patrolCount >= kMaxPatrolPoints) {
    return false;
  }

  _patrolPoints[_patrolCount] = point;
  _patrolCount++;
  return true;
}

void ActorMobile::Tick(World &world, ActorId self, std::chrono::milliseconds delta) {
  _state = PatrolState::Idle;
  // TODO
}

void ActorMobile::Draw(Painter &painter, const BitmapPosition &position, std::chrono::milliseconds animTime) const {
  // Only animate the walk cycle while actually moving; otherwise hold the first frame.
  Actor::Draw(painter, position, _state == PatrolState::Moving ? animTime : std::chrono::milliseconds(0));
}

const Actor::State &ActorMobile::GetCurrentState() const noexcept {
  return _spriteState;
}

}// namespace e00::impl
