#include "PrivateInclude.hpp"

namespace e00 {
World::World(std::string name, ResourcePtrT<e00::Map> &&map)
    : _name(std::move(name)),
      _map(std::move(map)) {
}

World::~World() = default;

World::NodeID World::Insert(Actor *actor, const Vec2D<WorldCoordinateType> &position) {
  // Is this actor in this world ?
  if (Size() < position) {
    return -1;
  }

  // Find first free element
  // TODO: replace this with a "NEXT FREE" type of system
  for (auto e = 0; e < _elements.size(); e++) {
    auto element = _elements.at(e);

    if (element.actor == nullptr) {
      element.actor = actor;
      element.position = position;
      return e;
    }
  }

  _elements.push_back({actor, position});
  return _elements.size() - 1;
}

void World::Update(NodeID element, const Vec2D<WorldCoordinateType> &position) {
  if (element < _elements.size()) {
    _elements.at(element).position = position;
  }
}

void World::Remove(NodeID element) {
  if (element < _elements.size()) {
    _elements.at(element).actor = nullptr;
    _elements.at(element).position = {};
  }
}

void World::ProcessAction(const ActionInstance &action) {
}

auto World::NumActors() const {
  return std::count_if(std::begin(_elements), std::end(_elements), [](const auto &element) {
    return element.actor != nullptr;
  });
}

std::vector<World::NodeID> &World::Query(const RectT<e00::WorldCoordinateType> &bounds, std::vector<World::NodeID> &output) const {

  // TODO This is horrible; make some kind of BSP
  for (NodeID i = 0; i < _elements.size(); i++) {
    const auto &element = _elements.at(i);
    if (bounds.Contains(element.position))
      output.push_back(i);
  }

  return output;
}

}// namespace e00
