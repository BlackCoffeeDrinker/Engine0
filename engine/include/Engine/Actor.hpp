#pragma once

#include <Engine/Config.hpp>
#include <Engine/Direction.hpp>
#include <Engine/Math/Vec2D.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Resource/Sprite.hpp>
#include <Engine/ResourcePtr.hpp>

#include <chrono>
#include <utility>

namespace e00 {
class Sprite;
class World;
class Stream;
class WritableStream;

/**
 * \brief Abstract class for all objects placed in a world
 *
 * Example of entities includes enemies, the hero,
 * non-playing characters, doors, chests, etc.
 */
class Actor {
public:
  enum class BodyType {
    Static, // Unmovable actor
    Dynamic,// Actor can move
    None,   // Invisible actor, used for tiggers
  };


  struct State {
    uint32_t stateId;
    std::array<ResourcePtrT<Sprite>, 4> sprite;
  };

private:
  Vec2D<WorldCoordinateType> _size;   // << Size of this actor
  WorldPosition _world_position{0, 0};// << Position in the world
  Direction _facing{Direction::South};// << Sprite direction to use
  BodyType _type{BodyType::Static};   // << Physics type

public:
  virtual ~Actor() = default;

  [[nodiscard]] virtual type_t Type() const = 0;

  /**
   * Renders this actor at `position`. Kept a simple, self-contained rendering function:
   * `animTime` is supplied by `World::Paint` from the element's stored facing and the
   * world's shared elapsed-time clock, so `Draw` needs no `World`/`NodeID` access.
   *
   * @param painter the painter to draw with
   * @param position screen-space position to draw at
   * @param animTime the world's current elapsed time, used to select the sprite's animation frame
   */
  virtual void Draw(Painter &painter, const BitmapPosition &position, std::chrono::milliseconds animTime) const {
    const auto &state = GetCurrentState();
    const auto &sprite = state.sprite[DirectionSpriteIndex(_facing)];
    if (!sprite) { return; }

    sprite->SetCurrentTime(animTime);
    sprite->Paint(painter, position);
  }

  virtual void Tick(World &world, ActorId self, std::chrono::milliseconds delta) {}

  /**
   * Called when something interacts with this actor (e.g. the player pressing an action button while near it).
   *
   * @param world the world this actor lives in
   * @param self this actor's node in the world
   * @return true if the interaction was handled
   */
  virtual bool OnInteract(World &world, ActorId self) { return false; }

  void SetFacing(Direction facing) noexcept { _facing = facing; }
  void SetSize(const Vec2D<WorldCoordinateType> &newSize) { _size = newSize; }
  void SetPosition(const WorldPosition &newPosition) { _world_position = newPosition; }

  [[nodiscard]] auto Size() const { return _size; }
  [[nodiscard]] auto PhysicsType() const noexcept { return _type; }
  [[nodiscard]] auto Facing() const noexcept { return _facing; }
  [[nodiscard]] auto Position() const noexcept { return _world_position; }

  /**
   * @return true if this actor has small per-instance state that must survive a world unload/reload
   */
  [[nodiscard]] virtual bool HasSavableState() const noexcept { return false; }

  /**
   * Writes this actor's small per-instance state to `out`. Only called when `HasSavableState()` is true.
   *
   * @param out the stream to write to
   * @return true on success
   */
  virtual bool SaveState(WritableStream &out) const { return true; }

  /**
   * Reads this actor's small per-instance state from `in`. Only called when `HasSavableState()` is true.
   *
   * @param in the stream to read from
   * @return true on success
   */
  virtual bool LoadState(Stream &in) { return true; }


  /**
   * @return this actor's currently active `State` (which sprite set applies right now)
   */
  [[nodiscard]] const State &CurrentState() const noexcept { return GetCurrentState(); }

  /**
   * Sets the sprite shown for `direction` while this actor is in the state identified by
   * `stateId`. Diagonal directions collapse to the nearest cardinal one. No-op if this actor
   * doesn't have a state matching `stateId`.
   *
   * @param stateId the state to modify
   * @param direction the direction this sprite should be shown for
   * @param sprite the sprite to show
   */
  void SetStateSprite(uint32_t stateId, Direction direction, ResourcePtrT<Sprite> sprite) noexcept {
    if (auto *state = GetMutableState(stateId)) {
      state->sprite[DirectionSpriteIndex(direction)] = std::move(sprite);
    }
  }

protected:
  void SetType(BodyType newType) noexcept { _type = newType; }
  [[nodiscard]] virtual const State &GetCurrentState() const noexcept = 0;

  /**
   * @return a mutable pointer to this actor's `State` matching `stateId`, or `nullptr` if this
   * actor has no such state. Used by `SetStateSprite`; actors with per-instance states (chests,
   * mobiles, ...) should override this.
   */
  [[nodiscard]] virtual State *GetMutableState(uint32_t stateId) noexcept {
    (void) stateId;
    return nullptr;
  }

  /**
   * Maps an 8-way `Direction` to the index into `State::sprite` used by `Draw`.
   * Diagonal directions collapse to the nearest cardinal one (0=North, 1=East, 2=South, 3=West).
   */
  [[nodiscard]] static size_t DirectionSpriteIndex(Direction facing) noexcept {
    switch (CollapseTo4Way(facing)) {
      case Direction::North: return 0;
      case Direction::East: return 1;
      case Direction::South: return 2;
      case Direction::West: return 3;
      default: return 2;// unreachable for cardinal directions, fall back to South
    }
  }
};
}// namespace e00
