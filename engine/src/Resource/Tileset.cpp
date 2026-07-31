
#include "PrivateInclude.hpp"

#include "Painter_PaintDevice.hpp"

#include <memory>
#include <utility>

namespace e00 {

void Tileset::ComputeVolatileValues() {
  if (_tile_size.x != 0 && _tile_size.y != 0 && _source_sheet != nullptr) {
    _tiles_per_row = (_source_sheet->Size().x - _margin) / (_tile_size.x + _spacing);
  }
}

void Tileset::Paint(Painter &painter, TileIdType tileId, const Vec2D<BitmapSizeType> &position) {
  if (tileId > 0) [[likely]] {
    assert(_tiles_per_row != 0 && _source_sheet != nullptr);

    // We need to cut this tile from its tile sheet
    const auto tileX = tileId % _tiles_per_row;
    const auto tileY = tileId / _tiles_per_row;

    const auto sourceX = static_cast<BitmapSizeType>(_margin + tileX * (_tile_size.x + _spacing));
    const auto sourceY = static_cast<BitmapSizeType>(_margin + tileY * (_tile_size.y + _spacing));

    painter.BlitSurface(*_source_sheet, {sourceX, sourceY, _tile_size.x, _tile_size.y}, position);
  }
}

}// namespace e00
