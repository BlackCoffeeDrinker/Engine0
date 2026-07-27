#pragma once

#include <chrono>
#include <limits>

#include <Engine/Resource/Map.hpp>

namespace e00 {
struct Element {
  Actor *actor{nullptr};
  Vec2D<WorldCoordinateType> position{0, 0};

  [[nodiscard]] RectT<WorldCoordinateType> bounds() const {
    assert(actor != nullptr);
    return {position, actor->Size()};
  }

  [[nodiscard]] WorldCoordinateType DistanceTo(const Vec2D<WorldCoordinateType> &point) const {
    return position.DistanceTo(point);
  }
};

/**
 * A `World` is a collection of actors at a position with a state
 * Currently can only contain one map, but expected to be able to support many
 */
class World {
  std::string _name;
  ResourcePtrT<Map> _map;
  std::array<Element, detail::MaxActorsInWorld> _elements;

public:
  using NodeID = decltype(_elements)::size_type;
  static constexpr NodeID InvalidNodeID = std::numeric_limits<NodeID>::max();

  explicit World(std::string name);

  ~World();

  // Currently only supports one map
  std::error_code AddMap(ResourcePtrT<e00::Map> &&map) {
    _map = std::move(map);
    return {};
  }

  [[nodiscard]] auto Size() const { return _map->Size(); }
  [[nodiscard]] auto Width() const { return Size().x; }
  [[nodiscard]] auto Height() const { return Size().y; }
  [[nodiscard]] auto TileSize() const { return _map->TileSize(); }
  [[nodiscard]] const ResourcePtrT<Map> &Map() const { return _map; }
  [[nodiscard]] const auto &Actors() const { return _elements; }
  [[nodiscard]] size_t NumActors() const;

  void PaintMap(const Position &start, const Position &end, Painter &painter, const BitmapSize &painterOrigin) const {
    _map->PaintMap({start, end}, painter, painterOrigin);
  }

  std::vector<NodeID> &Query(const RectT<WorldCoordinateType> &bounds, std::vector<NodeID> &output) const;
  NodeID Insert(Actor *actor, const Vec2D<WorldCoordinateType> &position);
  void Update(NodeID element, const Vec2D<WorldCoordinateType> &position);
  void Remove(NodeID element);
  void Tick(std::chrono::milliseconds delta);
  bool ProcessAction(const ActionInstance &action);
};
}// namespace e00
