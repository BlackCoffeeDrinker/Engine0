#include "PrivateInclude.hpp"

namespace e00 {
World::World(std::string name, TilePosition mapSize)
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
      const TilePosition tilePos = {x, y};
      const auto mapIndex = PositionToLinear(TilePosition{
          static_cast<WorldCoordinateType>(rect.origin.x + tilePos.x),
          static_cast<WorldCoordinateType>(rect.origin.y + tilePos.y)});
      const auto tileId = layer.map_tiles.at(mapIndex);

      if (tileId == 0) {
        continue;
      }

      const auto &tileset = GetTilesetSource(tileId);
      const auto tileSize = tileset.tileset->TileSize();
      const BitmapPosition drawPos{
          static_cast<BitmapSizeType>(tilePos.x * tileSize.x + painterOrigin.x),
          static_cast<BitmapSizeType>(tilePos.y * tileSize.y + painterOrigin.y)};
      tileset.tileset->Paint(painter, tileId - tileset.firstStartTileId, drawPos);
    }
  }
}

ActorId World::Insert(ActorId actorId, Actor *actor, const WorldPosition &position) {
  // Is this actor in this world ?
  if (WorldPixelSize() < position) {
    return InvalidNodeID;
  }

  if (const auto it = _elements.insert(actorId, {.actor = actor}); it.second) {
    return actorId;
  }

  return InvalidNodeID;
}

void World::Update(ActorId element, const WorldPosition &position) {
  if (_elements.size() > element) {
    if (WorldPixelSize() < position) {
      return;
    }

    if (const auto &value = _elements.find(element); value != _elements.end()) {
      value->second.actor->SetPosition(position);
    }
  }
}

void World::Remove(ActorId element) {
  _elements.erase(element);
}

void World::Tick(std::chrono::milliseconds delta) {
  _elapsedTime += delta;

  // Move actors; make sure they don't move into anything that has collision

  for (auto &element: _elements) {
    if (element.second.actor->PhysicsType() == Actor::BodyType::Dynamic) {
      element.second.actor->Tick(*this, element.first, delta);
    }
  }
}

bool World::ProcessAction(const ActionInstance &action) {
  return false;
}

WorldPosition World::PositionOf(ActorId element) const {
  if (const auto it = _elements.find(element); it != _elements.end()) {
    return it->second.Position();
  }

  return {};
}

bool World::Interact(ActorId element) {
  if (const auto it = _elements.find(element); it != _elements.end()) {
    return it->second.actor->OnInteract(*this, element);
  }

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

void World::Paint(Painter &painter, const WorldPosition &start, const WorldPosition &end, const BitmapPosition &painterOrigin) const {
  const WorldRect worldBounds = WorldRect::FromPositions(start, end);

  // Figure out start & end in tiles
  const auto tileSize = _source_tileset.tileset->TileSize();
  const TilePosition tileStart = ToTilePosition(WorldPosition{start.x, start.y}, tileSize);

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
        const auto& tileset_source = GetTilesetSource(tileId);
        tileset_source.tileset->Paint(painter, tile - tileset_source.firstStartTileId, bitmapPos + painterOrigin);
      }
    }
  }

  // Actors
  for (const auto &[actorId, element]: _elements) {
    if (worldBounds.Contains(element.Position()))
      if (const auto *actor = _elements.at(actorId)->actor) {
        actor->Draw(painter, painterOrigin + (actor->Position() - start), {});
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
        const auto& tileset_source = GetTilesetSource(tileId);
        tileset_source.tileset->Paint(painter, tile - tileset_source.firstStartTileId, bitmapPos + painterOrigin);
      }
    }
  }
}

std::vector<ActorId> &World::Query(const WorldRect &bounds, std::vector<ActorId> &output) const {
  for (const auto &[actorId, element]: _elements) {
    if (bounds.Contains(element.Position()))
      output.push_back(actorId);
  }

  return output;
}

}// namespace e00
