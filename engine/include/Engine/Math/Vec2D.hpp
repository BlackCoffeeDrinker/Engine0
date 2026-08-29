#pragma once
#include "sqrt.hpp"

namespace e00 {
template<typename T, typename Tag = void>
struct Vec2D {
  T x;
  T y;

  constexpr static Vec2D min() {
    return Vec2D(std::numeric_limits<T>::min(), std::numeric_limits<T>::min());
  }

  constexpr static Vec2D max() {
    return Vec2D(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
  }

  constexpr Vec2D() noexcept : x{0}, y{0} {}
  constexpr Vec2D(T x_, T y_) noexcept : x{ x_ }, y{ y_ } {}

  [[nodiscard]] constexpr auto Area() const { return x * y; }

  [[nodiscard]] constexpr Vec2D Clamp(const Vec2D &maximums) const {
    return {
      x > maximums.x ? maximums.x : x,
      y > maximums.y ? maximums.y : y
    };
  }

  [[nodiscard]] T DistanceTo(const Vec2D &other) const {
    const auto dx = other.x - x;
    const auto dy = other.y - y;
    return detail::sqrt(static_cast<double>(dx * dx + dy * dy));
  }


  constexpr bool operator==(const Vec2D &rhs) const { return x == rhs.x && y == rhs.y; }
  constexpr bool operator!=(const Vec2D &rhs) const { return x != rhs.x || y != rhs.y; }
  constexpr bool operator>(const Vec2D &rhs) const { return x > rhs.x && y > rhs.y; }
  constexpr bool operator<(const Vec2D &rhs) const { return x < rhs.x && y < rhs.y; }

  constexpr Vec2D operator+(const Vec2D &rhs) const { return Vec2D(x + rhs.x, y + rhs.y); }
  constexpr Vec2D operator-(const Vec2D &rhs) const { return Vec2D(x - rhs.x, y - rhs.y); }
  constexpr Vec2D operator*(const Vec2D &rhs) const { return Vec2D(x * rhs.x, y * rhs.y); }
  constexpr Vec2D operator/(const Vec2D &rhs) const { return Vec2D(x / rhs.x, y / rhs.y); }

  constexpr Vec2D operator+(const T &rhs) const { return Vec2D(x + rhs, y + rhs); }
  constexpr Vec2D operator-(const T &rhs) const { return Vec2D(x - rhs, y - rhs); }
  constexpr Vec2D operator*(const T &rhs) const { return Vec2D(x * rhs, y * rhs); }
  constexpr Vec2D operator/(const T &rhs) const { return Vec2D(x / rhs, y / rhs); }
};

template<typename T, typename Tag>
constexpr Vec2D<T, Tag> min(const Vec2D<T, Tag> &lhs, const Vec2D<T, Tag> &rhs) {
  return {
    lhs.x < rhs.x ? lhs.x : rhs.x,
    lhs.y < rhs.y ? lhs.y : rhs.y
  };
}

template<typename T, typename Tag>
constexpr Vec2D<T, Tag> max(const Vec2D<T, Tag> &lhs, const Vec2D<T, Tag> &rhs) {
  return {
    lhs.x > rhs.x ? lhs.x : rhs.x,
    lhs.y > rhs.y ? lhs.y : rhs.y
  };
}

template<typename T, typename Tag>
T distance2(const Vec2D<T, Tag> &a, const Vec2D<T, Tag> &b) {
  const auto dx = b.x - a.x;
  const auto dy = b.y - a.y;
  return dx * dx + dy * dy;
}

template<typename T, typename Tag>
T distance(const Vec2D<T, Tag> &a, const Vec2D<T, Tag> &b) {
  return detail::sqrt(static_cast<double>(distance2(a, b)));
}

struct PixelTag {};
struct TileTag {};

using TilePosition = Vec2D<WorldCoordinateType, TileTag>; //< Tiles
using WorldPosition = Vec2D<WorldCoordinateType, PixelTag>; //< Pixels
using BitmapPosition = Vec2D<BitmapSizeType, PixelTag>; //< Pixels
using BitmapSize = Vec2D<BitmapSizeType>; //< Untagged extent (pixels or tiles depending on use)

constexpr WorldPosition ToWorldPosition(const TilePosition &tile, const BitmapSize &tileSizeInPixels) {
  return WorldPosition{
    static_cast<WorldCoordinateType>(tile.x * tileSizeInPixels.x),
    static_cast<WorldCoordinateType>(tile.y * tileSizeInPixels.y)};
}

constexpr TilePosition ToTilePosition(const WorldPosition &pixel, const BitmapSize &tileSizeInPixels) {
  return TilePosition{
    static_cast<WorldCoordinateType>(pixel.x / tileSizeInPixels.x),
    static_cast<WorldCoordinateType>(pixel.y / tileSizeInPixels.y)};
}

}// namespace e00
