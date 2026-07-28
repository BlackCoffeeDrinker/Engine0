#include "PrivateInclude.hpp"

namespace e00 {

RectT<BitmapSizeType> Map::SourceTileset::GetTileRect(TileIdType tile_id) const {
  assert(tiles_per_row != 0 && source_sheet != nullptr);

  // We need to cut this tile from its tile sheet
  const auto tileX = tile_id % tiles_per_row;
  const auto tileY = tile_id / tiles_per_row;

  const auto sourceX = static_cast<BitmapSizeType>(margin + tileX * (tile_size.x + spacing));
  const auto sourceY = static_cast<BitmapSizeType>(margin + tileY * (tile_size.y + spacing));

  return {sourceX, sourceY, tile_size.x, tile_size.y};
}

size_t Map::SourceTileset::GetTileBufferSize() const {
  return helpers::GetBufferSizeForBitmapSize(bmp_desc.bit_depth, tile_size.x, tile_size.y);
}

void Map::SourceTileset::ComputeVolatileValues() {
  if (tile_size.x != 0 && tile_size.y != 0 && source_sheet != nullptr) {
    tiles_per_row = (source_sheet->Size().x - margin) / (tile_size.x + spacing);

    bmp_desc = detail::SoftwareBitmapHelper(source_sheet->GetBitDepth(), tile_size);
    if (bmp_desc.bit_depth == DrawableSurface::BitDepth::DEPTH_8) {
      bmp_desc.bit_depth = DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE;
    }

    target_info = bmp_desc.GetTargetInformation();
  }
}

void Map::LoadTileId(TileIdType tile_id) {
  if (const auto it = _tilesets.find(tile_id); it == _tilesets.end()) {
    const auto &sourceSet = GetTilesetSource(tile_id);
    const auto sourceRect = sourceSet.GetTileRect(tile_id - sourceSet.start_tile_id);

    auto [place, emplaced] = _tilesets.emplace(tile_id, std::vector<uint8_t>());
    auto &tile = place->second;
    tile.resize(sourceSet.GetTileBufferSize());

    for (BitmapSizeType y = 0; y != sourceRect.size.y; ++y) {
      sourceSet.source_sheet->ReadLineInto(
          sourceRect.origin.y + y,
          sourceRect.origin.x + 0, sourceRect.origin.x + sourceRect.size.x,
          sourceSet.target_info,
          sourceSet.bmp_desc.GetLineData(std::span(tile), y));
    }
  }
}

size_t Map::SizeUsage() {
  return sizeof(*this);
}

void Map::SetTileset(ResourcePtrT<DrawableResource> set) {
  // Make sure the tileset is valid & is compatible if we already have one
  if (!set) { return; }

  _source_tilesets.SetTileset(set);
  _palette = FixedPalette(set->GetNumberOfColorsInPalette());
  for (auto idx = 0; idx < _palette.size(); ++idx) {
    _palette[idx] = set->GetColorFromPalette(idx);
  }
}

void Map::SetTileSize(const Vec2D<BitmapSizeType> &size) {
  _source_tilesets.SetTileSize(size);
}

void Map::renderTileId(Painter &painter, const BitmapSize &painter_origin, const Vec2D<unsigned short> tilePos, const std::vector<unsigned short>::value_type tileId) {
  assert(tileId != 0);

  const auto &sourceSet = GetTilesetSource(tileId);
  const auto &it = _tilesets.find(tileId);
  
  assert(it != _tilesets.end() && !it->second.empty());

  const Vec2D<BitmapSizeType> origin = tilePos * sourceSet.tile_size + painter_origin;

  for (BitmapSizeType bitmap_y = 0; bitmap_y < sourceSet.tile_size.y; bitmap_y++) {
    painter.BlitRawLine(
        origin.y + bitmap_y,
        origin.x, origin.x + sourceSet.tile_size.x,
        sourceSet.bmp_desc.GetLineData(std::span(it->second), bitmap_y),
        sourceSet.bmp_desc.GetTargetInformation());
  }
}

void Map::PaintGround(const RectT<WorldCoordinateType> &rect, Painter &painter, const BitmapSize &painter_origin) {
  {
    const auto start = rect.From();
    const auto end = rect.To();

    std::set<WorldCoordinateType> tiles;

    for (WorldCoordinateType y = start.y; y < end.y; y++) {
      for (WorldCoordinateType x = start.x; x < end.x; x++) {
        const auto mapIndex = PositionToLinear({x, y});
        assert(ValidDataPosition(mapIndex));

        if (TileIdType tile = _map_tiles[mapIndex]; tile > 0) {
          tiles.insert(tile);
        }
      }
    }

    for (const auto &tile: tiles) {
      LoadTileId(tile);
    }
  }

  for (WorldCoordinateType y = 0; y < rect.size.y; y++) {
    for (WorldCoordinateType x = 0; x < rect.size.x; x++) {
      const Vec2D tilePos = {x, y};
      const auto mapIndex = PositionToLinear(rect.origin + tilePos);
      const auto tileId = _map_tiles.at(mapIndex);
      if (tileId == 0) {
        continue;
      }

      renderTileId(painter, painter_origin, tilePos, tileId);
    }
  }
}

}// namespace e00
