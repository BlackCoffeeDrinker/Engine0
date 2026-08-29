#pragma once
#include "Engine/Actor.hpp"

#include <array>

namespace e00::impl {

/**
 * Class ActorDoor
 *
 * A static actor that blocks movement (`BodyType::Static`) while closed. Interacting with it
 * toggles it open/closed; once open it stops blocking (`BodyType::None`) so other actors can
 * walk through it.
 */
class ActorDoor : public Actor {
  bool _open{false};
  std::array<State, 2> _states{State{kClosedStateId, {}}, State{kOpenStateId, {}}};

public:
  static constexpr uint32_t kClosedStateId = 0;
  static constexpr uint32_t kOpenStateId = 1;

  ActorDoor();

  [[nodiscard]] type_t Type() const override { return type_id<ActorDoor>(); }

  bool OnInteract(World &world, ActorId self) override;

  [[nodiscard]] bool HasSavableState() const noexcept override { return true; }
  bool SaveState(WritableStream &out) const override;
  bool LoadState(Stream &in) override;

  [[nodiscard]] bool IsOpen() const noexcept { return _open; }

  /**
   * Sets the sprite shown (for every facing direction) while the door is closed.
   */
  void SetClosedSprite(ResourcePtrT<Sprite> sprite);

  /**
   * Sets the sprite shown (for every facing direction) while the door is open.
   */
  void SetOpenSprite(ResourcePtrT<Sprite> sprite);

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override { return _states[_open ? 1 : 0]; }

  [[nodiscard]] State *GetMutableState(uint32_t stateId) noexcept override {
    for (auto &state: _states) {
      if (state.stateId == stateId) {
        return &state;
      }
    }
    return nullptr;
  }

  /**
   * Applies `_open`'s blocking behavior to this actor's `BodyType`: `Static` (blocking) while
   * closed, `None` (non-blocking, passable) while open.
   */
  void UpdateBodyType() noexcept { SetType(_open ? BodyType::None : BodyType::Static); }
};

}// namespace e00::impl
