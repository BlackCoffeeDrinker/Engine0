#include "PrivateInclude.hpp"

namespace e00 {
World::World(std::string name, Vec2D<WorldCoordinateType> mapSize)
    : _map_size(mapSize),
      _name(std::move(name)),
      _source_tileset(),
      _ground_layer(LayerSize()),
      _above_actors_layer(LayerSize()) {
}

World::~World() = default;

void World::PaintLayer(const Layer &layer,
                       Painter &painter,
                       const RectT<TileIdType> &rect,
                       const BitmapSize &painterOrigin) const {
  for (WorldCoordinateType y = 0; y < rect.size.y; y++) {
    for (WorldCoordinateType x = 0; x < rect.size.x; x++) {
      const Vec2D tilePos = {x, y};
      const auto mapIndex = PositionToLinear(rect.origin + tilePos);
      const auto tileId = layer.map_tiles.at(mapIndex);

      if (tileId == 0) {
        continue;
      }

      const auto &tileset = GetTilesetSource(tileId);
      tileset.tileset->Paint(
          painter,
          tileId - tileset.firstStartTileId,
          tilePos * tileset.tileset->TileSize() + painterOrigin);
    }
  }
}

void World::GridInit(size_t widthInTiles, size_t heightInTiles) {
  _gridWidth = (widthInTiles + _gridCellSizeInTiles - 1) / _gridCellSizeInTiles;
  _gridHeight = (heightInTiles + _gridCellSizeInTiles - 1) / _gridCellSizeInTiles;
  if (_gridWidth == 0) {
    _gridWidth = 1;
  }
  if (_gridHeight == 0) {
    _gridHeight = 1;
  }
  _cellHeads.assign(_gridWidth * _gridHeight, std::numeric_limits<uint16_t>::max());
}

void World::GridClear() {
  std::ranges::fill(_cellHeads, std::numeric_limits<uint16_t>::max());
}

void World::GridInsert(NodeID index, const Vec2D<WorldCoordinateType> &position) {
  if (_cellHeads.empty()) {
    return;
  }

  const auto cx = static_cast<size_t>(position.x) / _gridCellSizeInTiles;
  const auto cy = static_cast<size_t>(position.y) / _gridCellSizeInTiles;

  if (cx < _gridWidth && cy < _gridHeight) {
    const auto cellIndex = cy * _gridWidth + cx;
    auto &element = _elements.at(index);
    element.nextInCell = _cellHeads[cellIndex];
    _cellHeads[cellIndex] = static_cast<uint16_t>(index);
  }
}

NodeID World::Insert(std::string name, Actor *actor, const Vec2D<WorldCoordinateType> &position) {
  // Is this actor in this world ?
  if (WorldTileSize() < position) {
    return InvalidNodeID;
  }

  // Find first `null` actor
  for (auto i = 0u; i < _elements.size(); i++) {
    auto &element = _elements.at(i);
    if (element.actor == nullptr) {
      element.name = std::move(name);
      element.actor = actor;
      element.position = position;

      return i;
    }
  }

  return InvalidNodeID;
}

void World::Update(NodeID element, const Vec2D<WorldCoordinateType> &position) {
  if (_elements.size() > element) {
    if (WorldTileSize() < position) {
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

void World::Tick(std::chrono::milliseconds delta) {
  GridClear();
  for (NodeID i = 0; i < _elements.size(); i++) {
    const auto &element = _elements.at(i);
    if (element.actor != nullptr) {
      GridInsert(i, element.position);
    }
  }

  for (auto &element: _elements) {
    if (element.actor != nullptr) {
      element.actor->Tick(delta);
    }
  }
}

bool World::ProcessAction(const ActionInstance &action) {
  return false;
}

bool World::AddTileset(TileIdType startTileId, const ResourcePtrT<Tileset> &tileset) {
  _source_tileset.firstStartTileId = startTileId;
  _source_tileset.tileset = tileset;
  return true;
}

bool World::SetMapTile(uint8_t layer, const TilePosition &position, TileIdType tileId) {
  if (const auto i = PositionToLinear(position);
      ValidDataPosition(i)) [[likely]] {
    if (layer == 0) {
      _ground_layer.map_tiles[i] = tileId;
    } else {
      _above_actors_layer.map_tiles[i] = tileId;
    }
    return true;
  }

  return false;
}

void World::Paint(Painter &painter, const BitmapPosition &start, const BitmapPosition &end, const BitmapSize &painterOrigin) const {
  const auto tileSize = _source_tileset.tileset->TileSize();

  // Figure out start & end in tiles
  const TilePosition tileStart = {static_cast<WorldCoordinateType>(start.x / tileSize.x), static_cast<WorldCoordinateType>(start.y / tileSize.y)};

  // Make sure we have somewhat valid coordinates
  if (tileStart > _map_size) {
    return;
  }

  // Clamp tileEnd to the max map
  const TilePosition tileEnd = {
      std::min(static_cast<WorldCoordinateType>(end.x / tileSize.x), _map_size.x),
      std::min(static_cast<WorldCoordinateType>(end.y / tileSize.y), _map_size.y)};

  const auto tileDrawSize = tileEnd - tileStart;

  // Do layer 0
  for (WorldCoordinateType y = 0; y < tileDrawSize.y; y++) {
    for (WorldCoordinateType x = 0; x < tileDrawSize.x; x++) {
      const TilePosition tilePos = {x, y};
      const BitmapPosition bitmapPos = {static_cast<BitmapSizeType>(tilePos.x * tileSize.x), static_cast<BitmapSizeType>(tilePos.y * tileSize.y)};
      const auto tileId = PositionToLinear(tilePos + tileStart);
      if (const auto tile = _ground_layer.map_tiles[tileId];
          tile != 0) {
        _source_tileset.tileset->Paint(painter, tile - _source_tileset.firstStartTileId, bitmapPos);
      }
    }
  }

  // Actors
  if (false && _gridWidth != 0 && _gridHeight != 0) {
    const auto minCx = static_cast<size_t>(start.x) / _gridCellSizeInTiles;
    const auto minCy = static_cast<size_t>(start.y) / _gridCellSizeInTiles;
    const auto maxCx = end.x == 0 ? 0 : (static_cast<size_t>(end.x) - 1) / _gridCellSizeInTiles;
    const auto maxCy = end.y == 0 ? 0 : (static_cast<size_t>(end.y) - 1) / _gridCellSizeInTiles;

    const auto lastCx = std::min<size_t>(maxCx, _gridWidth - 1);
    const auto lastCy = std::min<size_t>(maxCy, _gridHeight - 1);

    for (size_t cy = minCy; cy <= lastCy && cy < _gridHeight; cy++) {
      for (size_t cx = minCx; cx <= lastCx && cx < _gridWidth; cx++) {
        auto index = _cellHeads[cy * _gridWidth + cx];
        while (index != std::numeric_limits<uint16_t>::max()) {
          const auto &element = _elements.at(index);
          if (element.actor != nullptr && element.position > start && element.position < end) {
            const auto relativePosition = element.position - start;
            const auto drawPosition = painterOrigin + relativePosition;
            element.actor->Draw(painter, drawPosition);
          }

          index = element.nextInCell;
        }
      }
    }
  }

  // Do layer 1
  for (WorldCoordinateType y = 0; y < tileDrawSize.y; y++) {
    for (WorldCoordinateType x = 0; x < tileDrawSize.x; x++) {
      const TilePosition tilePos = {x, y};
      const BitmapPosition bitmapPos = {static_cast<BitmapSizeType>(tilePos.x * tileSize.x), static_cast<BitmapSizeType>(tilePos.y * tileSize.y)};
      const auto tileId = PositionToLinear(tilePos + tileStart);
      if (const auto tile = _above_actors_layer.map_tiles[tileId];
          tile != 0) {
        _source_tileset.tileset->Paint(painter, tile - _source_tileset.firstStartTileId, bitmapPos);
      }
    }
  }
}

std::vector<NodeID> &World::Query(const RectT<WorldCoordinateType> &bounds, std::vector<NodeID> &output) const {
  if (_gridWidth == 0 || _gridHeight == 0) {
    return output;
  }

  const auto from = bounds.From();
  const auto to = bounds.To();

  const auto minCx = static_cast<size_t>(from.x) / _gridCellSizeInTiles;
  const auto minCy = static_cast<size_t>(from.y) / _gridCellSizeInTiles;
  const auto maxCx = to.x == 0 ? 0 : (static_cast<size_t>(to.x) - 1) / _gridCellSizeInTiles;
  const auto maxCy = to.y == 0 ? 0 : (static_cast<size_t>(to.y) - 1) / _gridCellSizeInTiles;

  const auto lastCx = std::min<size_t>(maxCx, _gridWidth - 1);
  const auto lastCy = std::min<size_t>(maxCy, _gridHeight - 1);

  for (size_t cy = minCy; cy <= lastCy && cy < _gridHeight; cy++) {
    for (size_t cx = minCx; cx <= lastCx && cx < _gridWidth; cx++) {
      auto index = _cellHeads[cy * _gridWidth + cx];
      while (index != std::numeric_limits<uint16_t>::max()) {
        const auto &element = _elements.at(index);
        if (element.actor != nullptr && bounds.Contains(element.position)) {
          output.push_back(index);
        }
        index = element.nextInCell;
      }
    }
  }

  return output;
}

}// namespace e00
