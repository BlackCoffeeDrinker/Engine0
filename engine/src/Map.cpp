#include "PrivateInclude.hpp"

namespace e00 {
void Map::LoadTileId(TileIdType tile_id) {
  assert(tile_id < _tilesets.size());
  assert(_source_sheet.tiles_per_row != 0 && _source_sheet.tileset != nullptr);

  auto &tile = _tilesets[tile_id];
  if (tile.empty()) {
    // We need to cut this tile from its tilesheet
    const auto tileX = tile_id % _source_sheet.tiles_per_row;
    const auto tileY = tile_id / _source_sheet.tiles_per_row;

    const auto sourceX = static_cast<BitmapSizeType>(_source_sheet.margin + tileX * (_tileset_tile_size.x + _source_sheet.spacing));
    const auto sourceY = static_cast<BitmapSizeType>(_source_sheet.margin + tileY * (_tileset_tile_size.y + _source_sheet.spacing));

    tile.resize(helpers::GetBufferSizeForBitmapSize(_bmp_desc.bit_depth, _tileset_tile_size.x, _tileset_tile_size.y));
    auto dst = _bmp_desc.GetTargetInformation();
    dst.palette = &_palette;

    for (BitmapSizeType y = 0; y != _tileset_tile_size.y; ++y) {
      _source_sheet.tileset->ReadLineInto(
          sourceY + y,
          sourceX + 0, sourceX + _tileset_tile_size.x,
          dst, _bmp_desc.GetLineData(tile, y));
    }
  }
}

size_t Map::SizeUsage() {
  return sizeof(*this);
}

void Map::SetTileset(ResourcePtrT<DrawableResource> set) {
  // Make sure the tileset is valid & is compatible if we already have one
  if (!set) { return; }

  _palette = FixedPalette(set->GetNumberOfColorsInPalette());
  for (auto idx = 0; idx < _palette.size(); ++idx) {
    _palette[idx] = set->GetColorFromPalette(idx);
  }

  _bmp_desc = detail::SoftwareBitmapHelper(set->GetBitDepth(), set->Size());
  _source_sheet.tileset = set;

  if (_tileset_tile_size.x != 0 && _tileset_tile_size.y != 0) {
    _source_sheet.tiles_per_row = (_source_sheet.tileset->Size().x - _source_sheet.margin) / (_tileset_tile_size.x + _source_sheet.spacing);
    _source_sheet.count_rows = (_source_sheet.tileset->Size().y - _source_sheet.margin) / (_tileset_tile_size.y + _source_sheet.spacing);
    _tilesets.resize(_tilesets.size() + _source_sheet.tiles_per_row * _source_sheet.count_rows);
  }
}

void Map::SetTileSize(const Vec2D<BitmapSizeType> &size) {
  _tileset_tile_size = size;

  if (_source_sheet.tileset) {
    _source_sheet.tiles_per_row = (_source_sheet.tileset->Size().x - _source_sheet.margin) / (_tileset_tile_size.x + _source_sheet.spacing);
    _source_sheet.count_rows = (_source_sheet.tileset->Size().y - _source_sheet.margin) / (_tileset_tile_size.y + _source_sheet.spacing);
    _tilesets.resize(_tilesets.size() + _source_sheet.tiles_per_row * _source_sheet.count_rows);
  }
}

void Map::PaintMap(const RectT<WorldCoordinateType> &rect, Painter &painter, const BitmapSize &painter_origin) {

  // Ensure every tile is loaded
  {
    const auto start = rect.From();
    const auto end = rect.To();
    std::set<WorldCoordinateType> tiles;

    for (WorldCoordinateType y = start.y; y < end.y; y++) {
      for (WorldCoordinateType x = start.x; x < end.x; x++) {
        const auto mapIndex = PositionToLinear({x, y});
        assert(ValidDataPosition(mapIndex));

        for (const auto &layer: _layers) {
          if (const auto tileId = layer.map_tiles[mapIndex];
              tileId != 0) {
            tiles.insert(tileId - 1);
          }
        }
      }
    }

    for (const auto &tile: tiles)
      LoadTileId(tile);
  }

  auto format = _bmp_desc.GetTargetInformation();
  if (format.bit_depth == DrawableSurface::BitDepth::DEPTH_8) {
    format.bit_depth = DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE;
  }

  for (WorldCoordinateType y = 0; y < rect.size.y; y++) {
    for (WorldCoordinateType x = 0; x < rect.size.x; x++) {
      const Vec2D tilePos = {x, y};

      const Vec2D<BitmapSizeType> origin = tilePos * _tileset_tile_size + painter_origin;
      const auto mapIndex = PositionToLinear(rect.origin + tilePos);

      for (const auto &layer: _layers) {
        if (const auto &tileId = layer.map_tiles.at(mapIndex);
            tileId != 0 && static_cast<size_t>(tileId - 1) < _tilesets.size()) {
          const auto &tile = _tilesets[tileId - 1];
          if (tile.empty()) {
            continue;
          }

          for (BitmapSizeType bitmap_y = 0; bitmap_y < _tileset_tile_size.y; bitmap_y++) {
            painter.BlitRawLine(
                origin.y + bitmap_y,
                origin.x, origin.x + _tileset_tile_size.x,
                _bmp_desc.GetLineData(tile, bitmap_y),
                format);
          }
        }
      }
    }
  }
}

}// namespace e00
