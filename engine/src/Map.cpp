#include "PrivateInclude.hpp"

namespace e00 {

void Map::ComputeTilesetTileSize() {
  if (auto* res = _tileset.get()) {
    if (_tileset_size.x != 0 && _tileset_size.y != 0) {
      _tiles_per_row = (res->Size().x - _margin) / (_tileset_size.x + _spacing);
    }
  }
}

void Map::SetTileset(ResourcePtrT<DrawableResource> set) {
  _tileset = std::move(set);
  ComputeTilesetTileSize();
}

void Map::SetTileSize(const Vec2D<BitmapSizeType> &size) {
  _tileset_size = size;
  ComputeTilesetTileSize();
}

void Map::PaintTile(const Position &position, Painter &painter, const Vec2D<BitmapSizeType> &origin) {
  if (!_tileset || _tiles_per_row == 0) {
    return;
  }

  const auto mapIndex = PositionToLinear(position);
  if (!ValidDataPosition(mapIndex)) {
    return;
  }

  const auto tileId = _map_tile.at(mapIndex);
  if (tileId == 0) {
    return;
  }

  const auto localTileId = tileId - 1;

  const auto tileX = localTileId % _tiles_per_row;
  const auto tileY = localTileId / _tiles_per_row;

  const auto sourceX = static_cast<BitmapSizeType>(_margin + tileX * (_tileset_size.x + _spacing));
  const auto sourceY = static_cast<BitmapSizeType>(_margin + tileY * (_tileset_size.y + _spacing));

  painter.DrawSurface(
      _tileset.Ref(),
      RectT{Vec2D{sourceX, sourceY}, _tileset_size},
      origin);
}

}// namespace e00
