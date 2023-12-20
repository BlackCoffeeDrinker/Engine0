#pragma once

namespace e00 {
class Tileset {
  struct Tile {
    ComponentRegistry component_container;
  };

  Vec2D<uint16_t> _tileset_size;
  std::vector<Tile> _tiles;
  ResourcePtrT<e00::Bitmap> _tileset;
  uint16_t _margin;
  uint16_t _spacing;

public:
  using TileIdType = uint16_t;

  Tileset() = default;

  explicit Tileset(TileIdType nbTiles);

  Tileset(const Tileset&);

  Tileset(Tileset&&) noexcept;

  ~Tileset();

  Tileset& operator=(const Tileset&);

  Tileset& operator=(Tileset&&) noexcept;

  explicit operator bool() const noexcept { return !_tiles.empty(); }

  const auto& TileSize() const { return _tileset_size; }

  template<typename T>
  auto GetComponent(TileIdType tileId) const -> decltype(_tiles.at(tileId).component_container.GetComponent<T>()) {
    return _tiles.at(tileId).component_container.template GetComponent<T>();
  }

  template<typename T, typename ...Args>
  T* CreateComponent(TileIdType tileId, Args&&...args) {
    return _tiles.at(tileId).component_container.template CreateComponent<T, Args...>(std::forward<Args>(args)...);
  }

  template<typename T>
  [[nodiscard]] bool HasComponent(TileIdType tileId) const {
    return _tiles.at(tileId).component_container.template HasComponent<T>();
  }

  template<typename T>
  void RemoveComponent(TileIdType tileId) {
    _tiles.at(tileId).component_container.template RemoveComponent<T>();
  }

  [[nodiscard]] uint16_t NumberOfTiles() const { return _tiles.size(); }

  void SetBitmap(ResourcePtrT<Bitmap> &&bitmap) { _tileset = std::move(bitmap); }

  void SetMargin(uint16_t margin) { _margin = margin; }

  void SetSpacing(uint16_t padding) { _spacing = padding; }

  void SetTilesize(const Vec2D<uint16_t>& tilesize) { _tileset_size = tilesize; }

  void DrawTile(TileIdType tileId, Bitmap& destination, const Vec2D<uint16_t>& position) const;

};
}
