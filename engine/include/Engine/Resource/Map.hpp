#pragma once

namespace e00 {
/**
 * Raw map data
 */
class Map : public Resource {
public:
  using Position = Vec2D<WorldCoordinateType>;

private:
  struct TileOptions {};

  Vec2D<WorldCoordinateType> _size;
  std::vector<Tileset::TileIdType> _data;
  std::vector<TileOptions> _options;
  Tileset _tileset;

  [[nodiscard]] WorldCoordinateType LayerSize() const { return _size.Area(); }

  [[nodiscard]] WorldCoordinateType PositionToLinear(const Position &pos) const {
    if (pos > _size) {
      return std::numeric_limits<WorldCoordinateType>::max();
    }

    return (pos.y * _size.x) + pos.x;
  }

  [[nodiscard]] bool ValidDataPosition(size_t position) const {
    return _data.size() > position;
  }

public:
  Map() : _size(0, 0), _tileset() {}

  Map(WorldCoordinateType width, WorldCoordinateType height)
    : _size(width, height),
      _data(LayerSize()),
      _options(LayerSize()),
      _tileset() {}

  Map(const Map &other) = default;

  Map(Map &&other) noexcept
    : _size(other._size),
      _data(std::move(other._data)),
      _options(std::move(other._options)),
      _tileset(std::move(other._tileset)) {}

  ~Map() override = default;

  Map &operator=(const Map &other) {
    if (&other != this) {
      _size = other._size;
      _data.clear();
      _data = other._data;
      _options.clear();
      _options = other._options;
      _tileset = other._tileset;
    }

    return *this;
  }

  Map &operator=(Map &&other) noexcept {
    if (&other != this) {
      std::swap(_size, other._size);
      _data.clear();
      _data.swap(other._data);
      _options.clear();
      _options.swap(other._options);
      _tileset = std::move(other._tileset);
    }

    return *this;
  }

  explicit operator bool() const noexcept { return _size.x > 0 && _size.y > 0; }

  void SetTileset(Tileset &&set) { _tileset = std::move(set); }

  [[nodiscard]] const Tileset &Tileset() const { return _tileset; }
  [[nodiscard]] WorldCoordinateType Width() const { return _size.x; }
  [[nodiscard]] WorldCoordinateType Height() const { return _size.y; }
  [[nodiscard]] Vec2D<WorldCoordinateType> Size() const { return _size; }

  [[nodiscard]] Tileset::TileIdType HighestTitleId() const {
    return *std::max_element(std::begin(_data), std::end(_data));
  }

  /**
   * Get the Tile ID for a given position
   *
   * @param position
   * @return tileId or 0 if out of bound
   */
  [[nodiscard]] Tileset::TileIdType Get(const Position &position) const {
    if (const auto i = PositionToLinear(position);
      ValidDataPosition(i)) {
      return _data.at(i);
    }

#ifndef NDEBUG
    abort();
#endif

    return std::numeric_limits<Tileset::TileIdType>::max();
  }

  /**
   *
   *
   * @param position
   * @param tileId
   * @return
   */
  bool Set(const Position &position, Tileset::TileIdType tileId) {
    const auto i = PositionToLinear(position);
    if (ValidDataPosition(i)) {
      _data[i] = tileId;
      return true;
    }

    return false;
  }
};
}// namespace e00
