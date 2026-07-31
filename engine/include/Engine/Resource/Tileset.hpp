
#pragma once

#include <Engine/ResourcePtr.hpp>

namespace e00 {
struct TileOptions {};

class Tileset : public Resource {
  ResourcePtrT<DrawableResource> _source_sheet{};
  uint16_t _margin{};
  uint16_t _spacing{};
  uint16_t _tiles_per_row{};
  Vec2D<BitmapSizeType> _tile_size{0, 0};
  std::unordered_map<TileIdType, TileOptions> _tile_options;

  void ComputeVolatileValues();

public:
  Tileset() = default;
  ~Tileset() override = default;

  [[nodiscard]] type_t Type() const override { return type_id<Tileset>(); }
  [[nodiscard]] size_t SizeUsage() override { return 0; }

  void SetTileset(ResourcePtrT<DrawableResource> set) {
    _source_sheet = std::move(set);
    ComputeVolatileValues();
  }

  void SetTileSize(const Vec2D<BitmapSizeType> &size) {
    _tile_size = size;
    ComputeVolatileValues();
  }

  [[nodiscard]] const Vec2D<BitmapSizeType> &TileSize() const { return _tile_size; }


  void Paint(Painter &painter, TileIdType tileId, const Vec2D<BitmapSizeType> &position);
  void SetMargin(BitmapSizeType margin) { _margin = margin; }
  void SetSpacing(BitmapSizeType spacing) { _spacing = spacing; }
};
}// namespace e00
