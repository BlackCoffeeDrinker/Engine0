#pragma once
#include "Engine/Actor.hpp"

#include <array>

namespace e00::impl {

/**
 * Class ActorChest
 *
 * A static, stateful actor that can be opened/closed by interacting with it.
 */
class ActorChest : public Actor {
  bool _open{false};
  std::array<State, 2> _states{
      State{.stateId = kClosedStateId, .sprite = {}},
      State{.stateId = kOpenStateId, .sprite = {}}};

public:
  static constexpr uint32_t kClosedStateId = 0;
  static constexpr uint32_t kOpenStateId = 1;

  ActorChest();

  [[nodiscard]] type_t Type() const override { return type_id<ActorChest>(); }

  bool OnInteract(World &world, ActorId self) override;

  [[nodiscard]] bool HasSavableState() const noexcept override { return true; }
  bool SaveState(WritableStream &out) const override;
  bool LoadState(Stream &in) override;

  [[nodiscard]] bool IsOpen() const noexcept { return _open; }

  /**
   * Sets the sprite shown (for every facing direction) while the chest is closed.
   */
  void SetClosedSprite(ResourcePtrT<Sprite> sprite);

  /**
   * Sets the sprite shown (for every facing direction) while the chest is open.
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
};

}// namespace e00::impl
