#pragma once

namespace e00 {
class Sprite;

/**
 * \brief Abstract class for all objects placed on a map.
 *
 * Example of entities include enemies, the hero,
 * non-playing characters, doors, chests, etc.
 *
 * At the most fundamental level, an Actor is any Object that
 * you can place in a level.
 */
class Actor : public ComponentRegistry {
public:
  Actor() = default;

  ~Actor() override = default;

  enum class BodyType {
    Static,// Unmovable actor (A interactable tree for example)
    Dynamic,// Actor can move
  };

private:
  Vec2D<WorldCoordinateType> _size;// << Size of this actor
  BodyType _type;

public:
  /**
   * Called when spawned
   */
  virtual void OnSpawn() {}

  /**
   *
   * @param delta
   */
  virtual void Tick(const std::chrono::milliseconds &delta) {}

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
