#pragma once

#include "Engine/Math/Vec2D.hpp"

namespace e00 {
template<typename T, typename Tag = void>
struct RectT {
  using value_type = T;

  Vec2D<T> origin;
  Vec2D<T> size;

  constexpr static RectT min() {
    return RectT(Vec2D<T>::min(), Vec2D<T>::min());
  }

  constexpr static RectT max() {
    return RectT(Vec2D<T>::max(), Vec2D<T>::max());
  }

  constexpr static RectT maxArea() {
    return RectT(Vec2D<T>::min(), Vec2D<T>::max());
  }

  constexpr RectT() noexcept : origin{}, size{} {}
  constexpr RectT(const RectT &other) noexcept : origin(other.origin), size(other.size) {}
  constexpr RectT(T x, T y, T width, T height) noexcept : origin(x, y), size(width, height) {}
  constexpr RectT(const Vec2D<T> &pos, const Vec2D<T> &size) noexcept : origin(pos), size(size) {}

  constexpr Vec2D<T, Tag> From() const { return origin; }
  constexpr Vec2D<T, Tag> To() const { return Vec2D<T>(origin.x + size.x, origin.y + size.y); }

  constexpr Vec2D<T, Tag> GetTopLeft() const { return From(); }
  constexpr Vec2D<T, Tag> GetTopRight() const { return {origin.x + size.x, origin.y}; }
  constexpr Vec2D<T, Tag> GetBottomLeft() const { return {origin.x, origin.y + size.y}; }
  constexpr Vec2D<T, Tag> GetBottomRight() const { return To(); }

  constexpr Vec2D<T, Tag> Size() const { return size; }
  constexpr Vec2D<T, Tag> Origin() const { return origin; }

  constexpr Vec2D<T, Tag> Center() const {
    return {
        (origin.x + size.x) / static_cast<T>(2),
        (origin.y + size.y) / static_cast<T>(2)};
  }

  constexpr bool Contains(const Vec2D<T, Tag> &point) const {
    return (point.x >= origin.x && point.y >= origin.y && point.x < origin.x + size.x && point.y < origin.y + size.y);
  }

  constexpr bool Contains(const RectT &r2) const {
    return (origin.x < r2.origin.x + r2.size.x && origin.x + size.x > r2.origin.x) && (origin.y < r2.origin.y + r2.size.y && origin.y + size.y > r2.origin.y);
  }

  [[nodiscard]] RectT Unite(const RectT &r2) const {
    const auto maxTo = e00::max(To(), r2.To());
    const auto minOrigin = e00::min(origin, r2.origin);

    return {
        minOrigin,
        {static_cast<T>(maxTo.x - minOrigin.x), static_cast<T>(maxTo.y - minOrigin.y)}};
  }

  [[nodiscard]] bool isValid() const {
    return size.x > 0 && size.y > 0;
  }

  constexpr static RectT FromPositions(const Vec2D<T, Tag> &from, const Vec2D<T, Tag> &to) {
    RectT r;
    if (to.x > from.x) {
      r.origin.x = from.x;
      r.size.x = to.x - from.x;
    } else {
      r.origin.x = to.x;
      r.size.x = from.x - to.x;
    }

    if (to.y > from.y) {
      r.origin.y = from.y;
      r.size.y = to.y - from.y;
    } else {
      r.origin.y = to.y;
      r.size.y = from.y - to.y;
    }

    return r;
  }
};

using TileRect = RectT<WorldCoordinateType, TileTag>;  //< Tiles
using WorldRect = RectT<WorldCoordinateType, PixelTag>;//< Pixels
using BitmapRect = RectT<BitmapSizeType, PixelTag>;    //< Pixels

}// namespace e00
