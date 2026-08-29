#pragma once
#include "Engine/Actor.hpp"
#include "Engine/Math/Vec2D.hpp"
#include "Engine/Resource/Sprite.hpp"
#include "Engine/ResourcePtr.hpp"
#include "Engine/World.hpp"

#include <array>
#include <cstdint>

namespace e00::impl {
static constexpr size_t kMaxPatrolPoints = 8;

/**
 * Class ActorMobile
 *
 * A freely-moving actor (not grid-locked) that can optionally cycle through a small,
 * fixed-size set of patrol waypoints. Movement/patrol state uses only inline/stack
 * memory: no heap allocation happens per-tick.
 */
class ActorMobile : public Actor {
public:
  enum class PatrolState {
    Idle,   // << no patrol points configured, or not moving
    Moving, // << currently moving toward the current waypoint
    Waiting,// << arrived at a waypoint, about to move to the next one
  };

  struct MovementData {
    uint8_t patrolIndex{0};
  };

private:
  std::array<WorldPosition, kMaxPatrolPoints> _patrolPoints{};
  uint8_t _patrolCount{0};
  WorldCoordinateType _speed{0};
  PatrolState _state{PatrolState::Idle};
  State _spriteState{0, {}};
  MovementData _movement{};

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override;

  [[nodiscard]] State *GetMutableState(uint32_t stateId) noexcept override {
    return _spriteState.stateId == stateId ? &_spriteState : nullptr;
  }

public:
  ActorMobile();

  [[nodiscard]] type_t Type() const override { return type_id<ActorMobile>(); }

  void Tick(World &world, ActorId self, std::chrono::milliseconds delta) override;

  void SetSpeed(WorldCoordinateType pixelsPerSecond) noexcept { _speed = pixelsPerSecond; }
  [[nodiscard]] auto Speed() const noexcept { return _speed; }

  /**
   * Adds a patrol waypoint. Ignored once `kMaxPatrolPoints` is reached.
   *
   * @param point the waypoint to add, in world (pixel) coordinates
   * @return true if the point was added, false if the patrol array is already full
   */
  bool AddPatrolPoint(const WorldPosition &point);

  void ClearPatrolPoints() noexcept { _patrolCount = 0; }

  [[nodiscard]] uint8_t PatrolPointCount() const noexcept { return _patrolCount; }

  [[nodiscard]] auto CurrentPatrolState() const noexcept { return _state; }

  void Draw(Painter &painter, const BitmapPosition &position, std::chrono::milliseconds animTime) const override;
};

}// namespace e00::impl
