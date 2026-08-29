#pragma once
#include "ActorDoor.hpp"

#include <string>

namespace e00::impl {

/**
 * Class ActorWarpDoor
 *
 * An `ActorDoor` that, once opened (activated) by interacting with it, is meant to load a
 * different world at a named entry point (e.g. a door leading to another map).
 */
class ActorWarpDoor : public ActorDoor {
  std::string _targetWorld;
  std::string _targetEntryName;
  bool _activated{false};

public:
  [[nodiscard]] type_t Type() const override { return type_id<ActorWarpDoor>(); }

  bool OnInteract(World &world, ActorId self) override;

  void SetTargetWorld(std::string worldName) noexcept { _targetWorld = std::move(worldName); }
  [[nodiscard]] const std::string &TargetWorld() const noexcept { return _targetWorld; }

  void SetTargetEntry(std::string entryName) noexcept { _targetEntryName = std::move(entryName); }
  [[nodiscard]] const std::string &TargetEntry() const noexcept { return _targetEntryName; }

  /**
   * @return true once this warp door has been opened/activated by `OnInteract` and is waiting
   * for the caller (Engine/game) to actually load `TargetWorld()`. Cleared by `ClearActivation`.
   */
  [[nodiscard]] bool IsActivated() const noexcept { return _activated; }

  /**
   * Clears the pending activation flag; the caller should call this once it has handled (or
   * decided not to handle) the pending world load.
   */
  void ClearActivation() noexcept { _activated = false; }
};

}// namespace e00::impl
