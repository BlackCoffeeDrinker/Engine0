#pragma once

#include <system_error>
#include <chrono>

#include "Actor.hpp"
#include "Engine/Resource/Map.hpp"
#include "Engine/Resource/Tileset.hpp"

namespace e00 {
/**
 * A `World` is a collection of actors at a position with a state
 */
class World {
public:
  using NodeID = size_t;

  struct Element {
    Actor* actor;
    Vec2D<WorldCoordinateType> position;

    [[nodiscard]] RectT<WorldCoordinateType> bounds() const {
      return { position, actor->Size() };
    }

    [[nodiscard]] WorldCoordinateType DistanceTo(const Vec2D<WorldCoordinateType>& point) const {
      return position.DistanceTo(point);
    }
  };

  class Window {
    friend World;

    const World *_world;
    Vec2D<WorldCoordinateType> _start;
    Vec2D<WorldCoordinateType> _size;

    Window(const World *world, const Vec2D<WorldCoordinateType>& start, const Vec2D<WorldCoordinateType>& size)
      : _world(world),
        _start(start),
        _size(size) {}

  public:
    [[nodiscard]] auto Width() const { return _size.x; }
    [[nodiscard]] auto Height() const { return _size.y; }

    [[nodiscard]] auto MapToReal(const Vec2D<WorldCoordinateType>& position) const {
      return position > _size ? _size : position + _start;
    }

    template<typename T>
    auto GetComponent(const Vec2D<WorldCoordinateType>& position) const {
      if (position > _size)
        return (T *)(nullptr);

      return _world->_map->Tileset().template GetComponent<T>(MapToReal(position));
    }
  };

private:
  std::string _name;
  ResourcePtrT<Map> _map;
  std::vector<Element> _elements;

  template<typename T>
  constexpr static T computeStartForCenter(const T &center, const T &wanted, const T &max) {
    const auto half = wanted / 2;
    if ((half + center) > max) {
      return max - wanted;
    } else if (half > center) {
      return 0;
    } else {
      return center - half;
    }
  }

public:
  explicit World(std::string name, ResourcePtrT<Map>&& map);

  ~World();

  [[nodiscard]] auto Size() const { return _map->Size(); }
  [[nodiscard]] auto Width() const { return Size().x; }
  [[nodiscard]] auto Height() const { return Size().y; }
  [[nodiscard]] const auto& Map() const { return _map; }
  [[nodiscard]] auto NumActors() const;
  [[nodiscard]] const auto& Actors() const { return _elements; }

  /**
   *
   * @param bounds
   * @param output
   * @return `output`
   */
  std::vector<NodeID>& Query(const RectT<WorldCoordinateType>& bounds, std::vector<NodeID>& output) const;

  /**
   *
   * @param center
   * @param size
   * @return
   */
  [[nodiscard]] Window MakeWindow(const Vec2D<WorldCoordinateType>& center, const Vec2D<WorldCoordinateType>& size) const {
    // Adjust the "viewport"; don't go over the map
    Vec2D const adjSize(size.Clamp(Size()));

    // Compute window
    Vec2D const start = {
      computeStartForCenter(center.x, adjSize.x, Width()),
      computeStartForCenter(center.y, adjSize.y, Height())
    };

    // Build world window
    return { this, start, adjSize };
  }

  /**
   * Inserts actor `actor` at position `position`
   *
   * @param actor the Actor to add to this world
   * @param position the initial position the actor is at
   * @return
   */
  NodeID Insert(Actor* actor, const Vec2D<WorldCoordinateType>& position);

  /**
   * Change the position of element
   *
   * @param element
   * @param position
   */
  void Update(NodeID element, const Vec2D<WorldCoordinateType>& position);

  /**
   * Remove element
   *
   * @param element
   */
  void Remove(NodeID element);
};
}// namespace e00
