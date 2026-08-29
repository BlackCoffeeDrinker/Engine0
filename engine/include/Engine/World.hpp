#pragma once

#include <cassert>
#include <chrono>
#include <limits>

#include <Engine/ActionInstance.hpp>
#include <Engine/Actor.hpp>
#include <Engine/Config.hpp>
#include <Engine/Detail/FixedMap.hpp>
#include <Engine/Math/Rect.hpp>
#include <Engine/Math/Vec2D.hpp>
#include <Engine/Resource/Tileset.hpp>
#include <Engine/ResourcePtr.hpp>

namespace e00 {

/**
 * A `World` is a collection of actors at a position with a state
 */
class World {
  struct SourceTileset {
    TileIdType firstStartTileId;
    ResourcePtrT<Tileset> tileset;
  };

  struct Layer {
    std::vector<TileIdType> map_tiles;

    explicit Layer(size_t size) : map_tiles(size) {}
  };

  struct Element {
    Actor *actor{nullptr};

    [[nodiscard]] RectT<WorldCoordinateType> bounds() const {
      assert(actor != nullptr);
      const auto &pos = actor->Position();
      return {Vec2D{pos.x, pos.y}, actor->Size()};
    }

    [[nodiscard]] WorldCoordinateType DistanceTo(const WorldPosition &point) const {
      assert(actor != nullptr);
      return actor->Position().DistanceTo(point);
    }

    void SetPosition(const WorldPosition &newPosition) { actor->SetPosition(newPosition); }
    [[nodiscard]] auto Position() const noexcept { return actor->Position(); }
    [[nodiscard]] auto Facing() const noexcept { return actor->Facing(); }
  };

  const TilePosition _map_size;
  const std::string _name;

  SourceTileset _source_tileset;
  Layer _ground_layer;
  Layer _above_actors_layer;

  FixedMap<ActorId, Element, MaxActorsInWorld> _elements;
  std::chrono::milliseconds _elapsedTime{0};// << Accumulated world time, used to drive sprite animation frame selection

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _map_size.Area(); }
  [[nodiscard]] bool ValidDataPosition(const size_t position) const { return LayerSize() > position; }
  [[nodiscard]] const SourceTileset &GetTilesetSource(TileIdType tileId) const {
    (void) tileId;
    return _source_tileset;
  }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const TilePosition &pos) const {
    if (pos > _map_size) [[unlikely]] {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _map_size.x) + pos.x;
  }

  void PaintLayer(const Layer &layer, Painter &painter, const RectT<TileIdType> &layerTileRect, const BitmapSize &painterOrigin) const;

public:
  static constexpr ActorId InvalidNodeID = std::numeric_limits<ActorId>::max();
  explicit World(std::string name, TilePosition mapSize);

  ~World();

  [[nodiscard]] const std::string &Name() const noexcept { return _name; }

  [[nodiscard]] auto WorldTileSize() const { return _map_size; }
  [[nodiscard]] WorldPosition WorldPixelSize() const {
    if (!_source_tileset.tileset) {
      return WorldPosition{_map_size.x, _map_size.y};
    }
    const auto &tileSize = _source_tileset.tileset->TileSize();
    if (tileSize.x == 0 || tileSize.y == 0) {
      return WorldPosition{_map_size.x, _map_size.y};
    }
    return ToWorldPosition(_map_size, tileSize);
  }

  bool AddTileset(TileIdType startTileId, const ResourcePtrT<Tileset> &tileset);
  bool SetMapTile(uint8_t layer, const TilePosition &position, TileIdType tileId);

  void Paint(Painter &painter, const WorldPosition &start, const WorldPosition &end, const BitmapPosition &painterOrigin) const;

  std::vector<ActorId> &Query(const WorldRect &bounds, std::vector<ActorId> &output) const;
  ActorId Insert(ActorId actorId, Actor *actor, const WorldPosition &position);
  void Update(ActorId element, const WorldPosition &position);
  void Remove(ActorId element);

  /**
   * @param element the node to query
   * @return the position of the given node, or {0, 0} if invalid
   */
  [[nodiscard]] WorldPosition PositionOf(ActorId element) const;

  /**
   * @return the world's accumulated elapsed time, updated once per `Tick`; used to drive
   *         sprite animation frame selection at draw time.
   */
  [[nodiscard]] std::chrono::milliseconds ElapsedTime() const noexcept { return _elapsedTime; }

  /**
   * Dispatches an interaction to the actor at `element`.
   *
   * @param element the node to interact with
   * @return true if the interaction was handled
   */
  bool Interact(ActorId element);

  void Tick(std::chrono::milliseconds delta);
  bool ProcessAction(const ActionInstance &action);
};
}// namespace e00
