#pragma once

namespace e00 {
template<typename T>
struct AABB {
  Vec2D<T> minimum;
  Vec2D<T> maximum;
};

template<typename T>
AABB<T> merge(const AABB<T>& lhs, const AABB<T>& rhs) {
  return {
    min(lhs.minimum, rhs.minimum),
    max(lhs.maximum, rhs.maximum)
  };
}
}