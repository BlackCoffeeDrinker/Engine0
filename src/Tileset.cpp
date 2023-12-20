#include <Engine.hpp>
#include "Engine/Resource/Tileset.hpp"

namespace e00 {
Tileset::Tileset(TileIdType nbTiles)
  : _tileset_size({ 0, 0 }),
    _tiles(nbTiles),
    _tileset(nullptr),
    _margin(0),
    _spacing(0) {}

Tileset::~Tileset() = default;

Tileset::Tileset(const Tileset &other)
  : _tileset_size(other._tileset_size),
    _tiles(other._tiles),
    _tileset(other._tileset),
    _margin(other._margin),
    _spacing(other._spacing) {
}

Tileset::Tileset(Tileset &&rhs) noexcept
  : _tileset_size(rhs._tileset_size),
    _tiles(std::move(rhs._tiles)),
    _tileset(std::move(rhs._tileset)),
    _margin(rhs._margin),
    _spacing(rhs._spacing) {
}

Tileset &Tileset::operator=(const Tileset &rhs) {
  if (&rhs != this) {
    _tileset_size = rhs._tileset_size;
    _tiles = rhs._tiles;
    _tileset = rhs._tileset;
    _margin = rhs._margin;
    _spacing = rhs._spacing;
  }
  return *this;
}

Tileset &Tileset::operator=(Tileset &&rhs) noexcept {
  if (&rhs != this) {
    _tileset_size = rhs._tileset_size;
    _tiles = std::move(rhs._tiles);
    _tileset = std::move(rhs._tileset);
    _margin = rhs._margin;
    _spacing = rhs._spacing;
  }
  return *this;
}

void Tileset::DrawTile(Tileset::TileIdType tileId, Bitmap &destination, const Vec2D<uint16_t> &position) const {
  if (tileId >= _tiles.size()) {
    return;
  }

  // TODO: Optimize this
  // How many tiles per row ?
  const auto tiles_per_row = (_tileset->Size().x - _margin) / (_tileset_size.x + _spacing);

  // Compute source
  const auto x_pos = tileId / tiles_per_row;
  const auto y_pos = tileId % tiles_per_row;

  const auto x_spacing = x_pos > 0 ? (x_pos - 1) * _spacing : 0;
  const auto y_spacing = y_pos > 0 ? (y_pos - 1) * _spacing : 0;

  const Vec2D<uint16_t> source {
    static_cast<uint16_t>((x_pos * _tileset_size.x) + _margin + x_spacing),
    static_cast<uint16_t>((y_pos * _tileset_size.y) + _margin + y_spacing)
  };

  destination.Blit(_tileset.Ref(), {source, _tileset_size}, position);
}

}// namespace e00
