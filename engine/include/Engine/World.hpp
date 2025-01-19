#pragma once

#include <system_error>
#include <chrono>

#include "Actor.hpp"
#include "Engine.hpp"
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


private:
  std::string _name;
  ResourcePtrT<Map> _map;
  std::vector<Element> _elements;

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

  void ProcessAction(const ActionInstance & action);
};
}// namespace e00
