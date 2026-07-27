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

  const Vec2D<WorldCoordinateType> _map_size;
  std::vector<TileOptions> _options;

  struct Layer {
    std::vector<TileIdType> map_tiles;
  };
  struct SourceSheet {
    ResourcePtrT<DrawableResource> tileset{};
    uint16_t margin{};
    uint16_t spacing{};
    uint16_t tiles_per_row{};
    uint16_t count_rows{};
  };

  // region Tileset Bitmap
  detail::SoftwareBitmapHelper _bmp_desc;        //< Bitmap helper for `_tilesets`
  Vec2D<BitmapSizeType> _tileset_tile_size{0, 0};// Size of a tile in the tileset
  FixedPalette _palette{0};                      //< Palette of the tiles
  std::vector<std::vector<uint8_t>> _tilesets{}; //< Bitmap data of the tiles
  SourceSheet _source_sheet{};                   //< Currently only one source sheet
  // endregion

  // region Layer data
  std::vector<Layer> _layers{};
  // endregion

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _map_size.Area(); }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const Position &pos) const {
    if (pos > _map_size) [[unlikely]] {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _map_size.x) + pos.x;
  }

  [[nodiscard]] bool ValidDataPosition(size_t position) const { return LayerSize() > position; }

  void LoadTileId(TileIdType tile_id);

public:
  Map() : _map_size(0, 0) {}
  Map(WorldCoordinateType width, WorldCoordinateType height)
      : _map_size(width, height),
        _options(LayerSize()) {}


  ~Map() override = default;

  [[nodiscard]] size_t SizeUsage() override;
  [[nodiscard]] type_t Type() const override { return type_id<Map>(); }
  explicit operator bool() const noexcept { return _map_size.x > 0 && _map_size.y > 0; }
  [[nodiscard]] const ResourcePtrT<DrawableResource> &Tileset() const { return _source_sheet.tileset; }
  [[nodiscard]] WorldCoordinateType Width() const { return _map_size.x; }
  [[nodiscard]] WorldCoordinateType Height() const { return _map_size.y; }
  [[nodiscard]] Vec2D<WorldCoordinateType> Size() const { return _map_size; }

  void SetLayerCount(uint16_t layerCount) {
    _layers.resize(layerCount);
    for (auto &layer : _layers) {
      layer.map_tiles.resize(LayerSize());
    }
  }
  [[nodiscard]] auto GetLayerCount() const { return _layers.size(); }

  void SetTileset(ResourcePtrT<DrawableResource> set);
  void SetTilesetSpacing(uint16_t spacing) { _source_sheet.spacing = spacing; }

  [[nodiscard]] const Vec2D<BitmapSizeType> &TileSize() const { return _tileset_tile_size; }
  void SetTileSize(const Vec2D<BitmapSizeType> &size);
  
  bool Set(uint8_t layer, const Position &position, TileIdType tileId) {
    if (const auto i = PositionToLinear(position);
        ValidDataPosition(i)) [[likely]] {
      if (layer < _layers.size()) {
        _layers[layer].map_tiles[i] = tileId;
        return true;
      }
    }

    return false;
  }

  void PaintMap(const RectT<WorldCoordinateType> &rect, Painter &painter, const BitmapSize &painter_origin);
};
}// namespace e00
