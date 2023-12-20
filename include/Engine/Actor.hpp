#pragma once

namespace e00 {
/**
 * \brief Abstract class for all objects placed on a map.
 *
 * Example of entities include enemies, the hero,
 * non-playing characters, doors, chests, etc.
 */
class Actor : public ComponentRegistry {
public:
  enum class BodyType {
    Static,// Unmovable actor
    Dynamic,// Actor can move
  };

private:
  Vec2D<WorldCoordinateType> _size;// << Size of this actor
  BodyType _type;

public:
  bool IsHero() const;

  void Size(const Vec2D<WorldCoordinateType> &newSize) { _size = newSize; }

  /**
   * Size of this actor
   * @return
   */
  [[nodiscard]] auto Size() const { return _size; }

  /**
   *
   * @return
   */
  [[nodiscard]] auto Type() const noexcept { return _type; }

  bool HasSprites() const;

  ResourcePtrT<Sprite> GetSprite(const std::string_view& name) const;

  void ClearSprites();

};
}// namespace e00
