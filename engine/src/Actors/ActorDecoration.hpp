#pragma once
#include "Engine/Actor.hpp"

namespace e00::impl {

/**
 * Class ActorDecoration
 *
 * A simple actor used to place a decorative prop in a world. Non-colliding
 * (`BodyType::None`) by default; `SetCollidable` can turn it into a blocking obstacle
 * (`BodyType::Static`) instead.
 */
class ActorDecoration : public Actor {
  State _state{kStateId, {}};

public:
  static constexpr uint32_t kStateId = 0;

  ActorDecoration();

  [[nodiscard]] type_t Type() const override { return type_id<ActorDecoration>(); }

  /**
   * @param collidable true to block movement (`BodyType::Static`), false to let other actors
   * walk through it (`BodyType::None`, the default)
   */
  void SetCollidable(bool collidable) noexcept { SetType(collidable ? BodyType::Static : BodyType::None); }
  [[nodiscard]] bool IsCollidable() const noexcept { return PhysicsType() == BodyType::Static; }

  /**
   * Sets the sprite shown (for every facing direction) for this decoration.
   */
  void SetSprite(ResourcePtrT<Sprite> sprite);

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override { return _state; }

  [[nodiscard]] State *GetMutableState(uint32_t stateId) noexcept override {
    return _state.stateId == stateId ? &_state : nullptr;
  }
};

}// namespace e00::impl
