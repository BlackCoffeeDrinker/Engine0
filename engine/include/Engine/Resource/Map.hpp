#pragma once

#include <Engine/ResourcePtr.hpp>

#include <Engine/Resource/DrawableResource.hpp>

namespace e00 {
class Painter;
/**
 * Raw map data
 */
class Map : public Resource {
  struct TileOptions {};

  Vec2D<WorldCoordinateType> _map_size;
  std::vector<TileIdType> _map_tile;
  std::vector<TileOptions> _options;

  Vec2D<uint16_t> _tileset_size;
  ResourcePtrT<DrawableResource> _tileset{};
  uint16_t _margin{};
  uint16_t _spacing{};

  uint16_t _tiles_per_row{};

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _map_size.Area(); }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const Position &pos) const {
    if (pos > _map_size) {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _map_size.x) + pos.x;
  }

  [[nodiscard]] bool ValidDataPosition(size_t position) const { return _map_tile.size() > position; }
  void ComputeTilesetTileSize();

public:
  Map() : _map_size(0, 0), _tileset() {}

  Map(WorldCoordinateType width, WorldCoordinateType height)
      : _map_size(width, height),
        _map_tile(LayerSize()),
        _options(LayerSize()) {}

  Map(const Map &other) = default;

  Map(Map &&other) noexcept
      : _map_size(other._map_size),
        _map_tile(std::move(other._map_tile)),
        _options(std::move(other._options)),
        _tileset(std::move(other._tileset)) {}

  ~Map() override = default;

  Map &operator=(const Map &other) {
    if (&other != this) {
      _map_size = other._map_size;
      _map_tile.clear();
      _map_tile = other._map_tile;
      _options.clear();
      _options = other._options;
      _tileset = other._tileset;
    }

    return *this;
  }

  Map &operator=(Map &&other) noexcept {
    if (&other != this) {
      std::swap(_map_size, other._map_size);
      _map_tile.clear();
      _map_tile.swap(other._map_tile);
      _options.clear();
      _options.swap(other._options);
      _tileset = std::move(other._tileset);
    }

    return *this;
  }

  [[nodiscard]] type_t Type() const override { return type_id<Map>(); }
  explicit operator bool() const noexcept { return _map_size.x > 0 && _map_size.y > 0; }
  void SetTileset(ResourcePtrT<DrawableResource> set);
  [[nodiscard]] const ResourcePtrT<DrawableResource> &Tileset() const { return _tileset; }
  [[nodiscard]] WorldCoordinateType Width() const { return _map_size.x; }
  [[nodiscard]] WorldCoordinateType Height() const { return _map_size.y; }
  [[nodiscard]] Vec2D<WorldCoordinateType> Size() const { return _map_size; }
  [[nodiscard]] TileIdType HighestTitleId() const { return *std::ranges::max_element(_map_tile); }

  /**
   * 
   * @return size, in pixels, of a tile
   */
  [[nodiscard]] const Vec2D<BitmapSizeType> &TileSize() const { return _tileset_size; }
  void SetTileSize(const Vec2D<BitmapSizeType> &size);

  /**
   * Get the Tile ID for a given position
   *
   * @param position
   * @return tileId or 0 if out of bound
   */
  [[nodiscard]] TileIdType Get(const Position &position) const {
    if (const auto i = PositionToLinear(position);
        ValidDataPosition(i)) {
      return _map_tile.at(i);
    }

#ifndef NDEBUG
    abort();
#endif

    return std::numeric_limits<TileIdType>::max();
  }
  
  void SetTilesetSpacing(uint16_t spacing) { _spacing = spacing; }

  /**
   *
   *
   * @param position
   * @param tileId
   * @return
   */
  bool Set(const Position &position, TileIdType tileId) {
    const auto i = PositionToLinear(position);
    if (ValidDataPosition(i)) {
      _map_tile[i] = tileId;
      return true;
    }

    return false;
  }

  /**
   * Looks up the tile to use for world position `position` and paints it at
   * `origin` a bitmap of `tileSize`
   * 
   * @param position World position (tile position)
   * @param painter the painter to paint to
   * @param origin the start x, y to paint to
   */
  void PaintTile(const Position &position, Painter &painter, const Vec2D<BitmapSizeType> &origin);
};
}// namespace e00
