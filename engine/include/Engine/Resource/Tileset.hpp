#pragma once

namespace e00 {
class Tileset {
  struct Tile {
  };

  Vec2D<uint16_t> _tileset_size;
  std::vector<Tile> _tiles;
  ResourcePtrT<Bitmap> _tileset;
  uint16_t _margin;
  uint16_t _spacing;

public:
  using TileIdType = uint16_t;

  Tileset() = default;

  explicit Tileset(TileIdType nbTiles);

  Tileset(const Tileset &);

  Tileset(Tileset &&) noexcept;

  ~Tileset();

  Tileset &operator=(const Tileset &);

  Tileset &operator=(Tileset &&) noexcept;

  explicit operator bool() const noexcept { return !_tiles.empty(); }

  const auto &TileSize() const { return _tileset_size; }

  [[nodiscard]] uint16_t NumberOfTiles() const { return _tiles.size(); }

  void SetBitmap(ResourcePtrT<Bitmap> &&bitmap) { _tileset = std::move(bitmap); }

  void SetMargin(uint16_t margin) { _margin = margin; }

  void SetSpacing(uint16_t padding) { _spacing = padding; }

  void SetTilesize(const Vec2D<uint16_t> &tilesize) { _tileset_size = tilesize; }

  auto GetBitmap() const { return _tileset; }

  [[nodiscard]] RectT<uint16_t> GetTileRect(TileIdType tileId) const;

  // void DrawTile(TileIdType tileId, Bitmap& destination, const Vec2D<uint16_t>& position) const;
};
}// namespace e00
