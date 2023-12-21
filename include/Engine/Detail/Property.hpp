#pragma once

#include <functional>

namespace e00 {
/**
 * The Property template represents a value, with the additional functionality
 * that it can raise an event whenever that value is changed.
 *
 *
 * @tparam T
 */
template<class T>
struct Property {
public:
  Property() : value(), changed() {};
  Property(T val) : value(val), changed() {} // NOLINT(google-explicit-constructor)
  Property(T val, const std::function<void()> &changed) : value(val), changed(changed) {}
  Property(const Property<T> &val) : value(val.value), changed(val.changed) {}

  inline explicit operator const T &() const { return value; }
  inline const T *operator->() const { return &value; }
  inline bool operator==(const T &val) const { return value == val; }
  inline bool operator!=(const T &val) const { return value != val; }
  inline Property<T> &operator=(const Property<T> &val) {
    *this = val.value;
    return *this;
  }

  inline Property<T> &operator=(const T &val) {
    if (value != val) {
      value = val;
      if (changed) { changed(); }
    }

    return *this;
  }

private:
  std::function<void()> changed;
  T value;
};


}// namespace e00
