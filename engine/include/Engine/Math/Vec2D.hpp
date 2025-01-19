#pragma once

namespace e00 {
template<typename T>
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

  constexpr auto Area() const { return x * y; }

  constexpr Vec2D Clamp(const Vec2D &maximums) const {
    return {
      x > maximums.x ? maximums.x : x,
      y > maximums.y ? maximums.y : y
    };
  }

  T DistanceTo(const Vec2D &other) const {
    const auto dx = other.x - x;
    const auto dy = other.y - y;
    return std::sqrt(static_cast<double>(dx * dx + dy * dy));
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

template<typename T>
constexpr Vec2D<T> min(const Vec2D<T> &lhs, const Vec2D<T> &rhs) {
  return {
    lhs.x < rhs.x ? lhs.x : rhs.x,
    lhs.y < rhs.y ? lhs.y : rhs.y
  };
}

template<typename T>
constexpr Vec2D<T> max(const Vec2D<T> &lhs, const Vec2D<T> &rhs) {
  return {
    lhs.x > rhs.x ? lhs.x : rhs.x,
    lhs.y > rhs.y ? lhs.y : rhs.y
  };
}

template<typename T>
T distance2(const Vec2D<T> &a, const Vec2D<T> &b) {
  const auto dx = b.x - a.x;
  const auto dy = b.y - a.y;
  return dx * dx + dy * dy;
}

template<typename T>
T distance(const Vec2D<T> &a, const Vec2D<T> &b) {
  return std::sqrt(static_cast<double>(distance2(a, b)));
}
}// namespace e00
