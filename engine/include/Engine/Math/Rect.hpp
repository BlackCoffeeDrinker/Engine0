#pragma once

namespace e00 {
template<typename T>
struct RectT {
  Vec2D<T> origin;
  Vec2D<T> size;

  constexpr RectT() noexcept : origin{}, size{} {}
  constexpr RectT(const RectT &other) noexcept : origin(other.origin), size(other.size) {}
  constexpr RectT(T x, T y, T width, T height) noexcept : origin(x, y), size(width, height) {}
  constexpr RectT(const Vec2D<T> &pos, const Vec2D<T> &size) noexcept : origin(pos), size(size) {}

  constexpr Vec2D<T> From() const { return origin; }
  constexpr Vec2D<T> To() const { return Vec2D<T>(origin.x + size.x, origin.y + size.y); }

  constexpr Vec2D<T> GetTopLeft() const { return From(); }
  constexpr Vec2D<T> GetTopRight() const { return {origin.x + size.x, origin.y}; }
  constexpr Vec2D<T> GetBottomLeft() const { return {origin.x, origin.y + size.y}; }
  constexpr Vec2D<T> GetBottomRight() const { return To(); }

  constexpr Vec2D<T> Center() const {
    return {
      (origin.x + size.x) / static_cast<T>(2),
      (origin.y + size.y) / static_cast<T>(2)
    };
  }

  constexpr bool Contains(const Vec2D<T> &point) const {
    return (point.x >= origin.x && point.y >= origin.y
            && point.x < origin.x + size.x && point.y < origin.y + size.y);
  }

  constexpr bool Contains(const RectT<T> &r2) const {
    return (origin.x < r2.origin.x + r2.size.x && origin.x + size.x > r2.origin.x)
           && (origin.y < r2.origin.y + r2.size.y && origin.y + size.y > r2.origin.y);
  }


  constexpr static RectT FromPositions(const Vec2D<T> &from, const Vec2D<T> &to) {
    RectT<T> r;
    if (to.x > from.x) {
      r.origin.x = from.x;
      r.size.width = to.x - from.x;
    } else {
      r.origin.x = to.x;
      r.size.width = from.x - to.x;
    }

    if (to.y > from.y) {
      r.origin.y = from.y;
      r.size.height = to.y - from.y;
    } else {
      r.origin.y = to.y;
      r.size.height = from.y - to.y;
    }

    return r;
  }
};
}