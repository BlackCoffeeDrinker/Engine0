#include "PrivateInclude.hpp"

namespace e00 {
World::World(std::string name)
    : _name(std::move(name)),
      _map(nullptr) {
}

World::~World() = default;

World::NodeID World::Insert(Actor *actor, const Vec2D<WorldCoordinateType> &position) {
  // Is this actor in this world ?
  if (Size() < position) {
    return InvalidNodeID;
  }

  // Find first `null` actor
  for (auto i = 0u; i < _elements.size(); i++) {
    auto &element = _elements.at(i);
    if (element.actor == nullptr) {
      element.actor = actor;
      element.position = position;

      return i;
    }
  }

  return InvalidNodeID;
}

void World::Update(NodeID element, const Vec2D<WorldCoordinateType> &position) {
  if (_elements.size() > element) {
    if (Size() < position) {
      return;
    }

    auto &value = _elements.at(element);
    value.position = position;
    // Call updated position on actor ?
  }
}

void World::Remove(NodeID element) {
  if (element < _elements.size()) {
    auto &value = _elements.at(element);
    value.actor = nullptr;
    value.position = {};
  }
}

bool World::ProcessAction(const ActionInstance &action) {
}

size_t World::NumActors() const {
  return std::ranges::count_if(_elements, [](const auto &element) {
    return element.actor != nullptr;
  });
}

void World::PaintTile(const Position &tilePosition, Painter &painter, const Vec2D<BitmapSizeType> &origin) const {
  return _map->PaintTile(tilePosition, painter, origin);
}

std::vector<World::NodeID> &World::Query(const RectT<WorldCoordinateType> &bounds, std::vector<NodeID> &output) const {
  // TODO This is horrible; make some kind of BSP
  for (NodeID i = 0; i < _elements.size(); i++) {
    const auto &[actor, position] = _elements.at(i);
    if (actor != nullptr && bounds.Contains(position)) {
      output.push_back(i);
    }
  }

  return output;
}

}// namespace e00
