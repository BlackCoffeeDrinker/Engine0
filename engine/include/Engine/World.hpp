#pragma once

#include <cassert>
#include <chrono>
#include <limits>

#include <Engine/ActionInstance.hpp>
#include <Engine/Actor.hpp>
#include <Engine/Config.hpp>
#include <Engine/Math/Rect.hpp>
#include <Engine/Math/Vec2D.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Resource/Tileset.hpp>
#include <Engine/ResourcePtr.hpp>

namespace e00 {
using NodeID = size_t;

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
    Vec2D<WorldCoordinateType> position{0, 0};
    uint16_t nextInCell{std::numeric_limits<uint16_t>::max()};

    [[nodiscard]] RectT<WorldCoordinateType> bounds() const {
      assert(actor != nullptr);
      return {position, actor->Size()};
    }

    [[nodiscard]] WorldCoordinateType DistanceTo(const Vec2D<WorldCoordinateType> &point) const {
      return position.DistanceTo(point);
    }
  };

  const Vec2D<WorldCoordinateType> _map_size;
  std::string _name;

  SourceTileset _source_tileset;
  Layer _ground_layer;
  Layer _above_actors_layer;

  std::array<Element, detail::MaxActorsInWorld> _elements;

  static constexpr size_t _gridCellSizeInTiles = 4;
  size_t _gridWidth{0};
  size_t _gridHeight{0};
  std::vector<uint16_t> _cellHeads;

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _map_size.Area(); }
  [[nodiscard]] bool ValidDataPosition(const size_t position) const { return LayerSize() > position; }
  [[nodiscard]] const SourceTileset &GetTilesetSource(TileIdType tileId) const {
    (void) tileId;
    return _source_tileset;
  }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const WorldPosition &pos) const {
    if (pos > _map_size) [[unlikely]] {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _map_size.x) + pos.x;
  }

  void PaintLayer(const Layer &layer, Painter &painter, const RectT<TileIdType> &layerTileRect, const BitmapSize &painterOrigin) const;

  void GridInit(size_t widthInTiles, size_t heightInTiles);
  void GridClear();
  void GridInsert(NodeID index, const Vec2D<WorldCoordinateType> &position);

public:
  static constexpr NodeID InvalidNodeID = std::numeric_limits<NodeID>::max();
  explicit World(std::string name, Vec2D<WorldCoordinateType> mapSize);

  ~World();

  [[nodiscard]] auto WorldTileSize() const { return _map_size; }
  [[nodiscard]] auto WorldPixelSize() const { return _map_size * _source_tileset.tileset->TileSize(); }

  [[nodiscard]] size_t NumActors() const {
    return std::ranges::count_if(_elements, [](const auto &element) {
      return element.actor != nullptr;
    });
  }

  bool AddTileset(TileIdType startTileId, const ResourcePtrT<Tileset> &tileset);
  bool SetMapTile(uint8_t layer, const TilePosition &position, TileIdType tileId);

  void Paint(Painter &painter, const BitmapPosition &start, const BitmapPosition &end, const BitmapSize &painterOrigin) const;

  std::vector<NodeID> &Query(const RectT<WorldCoordinateType> &bounds, std::vector<NodeID> &output) const;
  NodeID Insert(Actor *actor, const WorldPosition &position);
  void Update(NodeID element, const WorldPosition &position);
  void Remove(NodeID element);

  void Tick(std::chrono::milliseconds delta);
  bool ProcessAction(const ActionInstance &action);
};
}// namespace e00
