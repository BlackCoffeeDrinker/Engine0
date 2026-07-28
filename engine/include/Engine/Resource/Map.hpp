#pragma once

#include <Engine/ResourcePtr.hpp>

#include <Engine/Resource/DrawableResource.hpp>
#include <utility>

namespace e00 {
class Painter;

/**
 * Raw map data
 */
class Map : public Resource {
  struct TileOptions {};
  
  struct SourceTileset {
    ResourcePtrT<DrawableResource> source_sheet{};
    uint16_t margin{};
    uint16_t spacing{};
    uint16_t tiles_per_row{};

    TileIdType start_tile_id{};

    Vec2D<BitmapSizeType> tile_size{0, 0};         // Size of a tile in the `tileset`
    DrawableSurface::TargetInformation target_info;//< Cached target information for `tilesets`
    detail::SoftwareBitmapHelper bmp_desc;         //< Bitmap helper for `tilesets`

    void SetTileset(ResourcePtrT<DrawableResource> set) {
      source_sheet = std::move(set);
      ComputeVolatileValues();
    }

    void SetTileSize(const Vec2D<BitmapSizeType> &size) {
      tile_size = size;
      ComputeVolatileValues();
    }

    [[nodiscard]] RectT<BitmapSizeType> GetTileRect(TileIdType tile_id) const;
    [[nodiscard]] size_t GetTileBufferSize() const;
  private:
    void ComputeVolatileValues();
  };

  const Vec2D<WorldCoordinateType> _map_size;
  std::vector<TileOptions> _options;
  FixedPalette _palette;

  SourceTileset _source_tilesets;
  std::map<TileIdType, std::vector<uint8_t>> _tilesets{};//< Bitmap data of the tiles
  std::vector<TileIdType> _map_tiles;

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _map_size.Area(); }
  [[nodiscard]] SourceTileset &GetTilesetSource(TileIdType tileId) {
    (void) tileId;
    return _source_tilesets;
  }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const Position &pos) const {
    if (pos > _map_size) [[unlikely]] {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _map_size.x) + pos.x;
  }

  [[nodiscard]] bool ValidDataPosition(size_t position) const { return LayerSize() > position; }

  void LoadTileId(TileIdType tile_id);

public:
  Map() : _map_size(0, 0), _source_tilesets() {}
  Map(WorldCoordinateType width, WorldCoordinateType height)
      : _map_size(width, height),
        _options(LayerSize()),
        _source_tilesets(),
        _map_tiles(LayerSize()) {}

  ~Map() override = default;

  [[nodiscard]] size_t SizeUsage() override;
  [[nodiscard]] type_t Type() const override { return type_id<Map>(); }
  explicit operator bool() const noexcept { return _map_size.x > 0 && _map_size.y > 0; }
  [[nodiscard]] WorldCoordinateType Width() const { return _map_size.x; }
  [[nodiscard]] WorldCoordinateType Height() const { return _map_size.y; }
  [[nodiscard]] Vec2D<WorldCoordinateType> Size() const { return _map_size; }
  [[nodiscard]] Vec2D<BitmapSizeType> TileSize() const { return _source_tilesets.tile_size; }

  void SetTileset(ResourcePtrT<DrawableResource> set);
  void SetTileSize(const Vec2D<BitmapSizeType> &size);
  void renderTileId(Painter &painter, const BitmapSize &painter_origin, Vec2D<unsigned short> tilePos, std::vector<unsigned short>::value_type tileId);
  void SetTilesetStartingTileId(TileIdType id) { _source_tilesets.start_tile_id = id; }
  void SetTilesetSpacing(uint16_t spacing) { _source_tilesets.spacing = spacing; }
  void SetTilesetMargin(uint16_t spacing) { _source_tilesets.margin = spacing; }

  bool SetGround(const Position &position, TileIdType tileId) {
    if (const auto i = PositionToLinear(position);
        ValidDataPosition(i)) [[likely]] {
      _map_tiles[i] = tileId;
      return true;
    }

    return false;
  }
  
  bool SetAbove(const Position &position, TileIdType tileId) {
    if (const auto i = PositionToLinear(position);
        ValidDataPosition(i)) [[likely]] {
      //_above[i * LayerSize()] = tileId;
      return true;
    }

    return false;
  }

  void PaintGround(const RectT<WorldCoordinateType> &rect, Painter &painter, const BitmapSize &painter_origin);
};
}// namespace e00
