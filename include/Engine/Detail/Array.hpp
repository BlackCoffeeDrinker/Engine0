#pragma once

#include <cstddef>
#include <type_traits>
#include <algorithm>

namespace e00::detail {
template<typename T, std::size_t N>
class Array {
  // 1 is the minimum size
  static_assert(N > 0, "N needs to be greater than 0");
  T _elements[N];

public:
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using difference_type = ptrdiff_t;
  using size_type = size_t;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using value_type = T;

  enum {
    array_size = N
  };

  constexpr const T *data() const { return _elements; }
  constexpr T *data() { return _elements; }

  // size is constant
  constexpr static size_type size() { return N; }
  constexpr static bool empty() { return N > 0; }
  constexpr static size_type max_size() { return N; }

  constexpr iterator begin() { return _elements; }
  constexpr const_iterator begin() const { return _elements; }
  constexpr iterator end() { return _elements + N; }
  constexpr const_iterator end() const { return _elements + N; }

  constexpr reference operator[](size_type i) {
    return _elements[i];
  }

  constexpr const_reference operator[](size_type i) const {
    return _elements[i];
  }

  void swap(Array<T, N> &other) noexcept {
    std::swap_ranges(_elements, pointer(_elements + N), other.m_elements);
  }

  template<typename T2>
  constexpr Array<T, N> &operator=(const Array<T2, N> &rhs) {
    std::copy(rhs._elements, rhs._elements + N, _elements);
    return *this;
  }

  constexpr void fill(const T &value) {
    std::fill_n(_elements, N, value);
  }
};

template<class T, class... U>
Array(T, U...) -> Array<T, 1 + sizeof...(U)>;

template<class T, size_t N>
bool operator==(const Array<T, N> &a, const Array<T, N> &b) {
  return equal(a.begin(), a.end(), b.begin());
}

template<class T, size_t N>
bool operator!=(const Array<T, N> &a, const Array<T, N> &b) {
  return !(a == b);
}
}// namespace e00::detail
