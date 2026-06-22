#pragma once

namespace e00 {
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

private:
  Vec2D<WorldCoordinateType> _size;// << Size of this actor
  BodyType _type{BodyType::Static};// << Physics type
  
public:
  virtual ~Actor() = default;

  void Size(const Vec2D<WorldCoordinateType> &newSize) { _size = newSize; }

  [[nodiscard]] auto Size() const { return _size; }

  [[nodiscard]] auto Type() const noexcept { return _type; }
};
}// namespace e00
