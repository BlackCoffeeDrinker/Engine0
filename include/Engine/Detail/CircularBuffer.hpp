#pragma once

#include <cstddef>
#include <type_traits>

#include "Array.hpp"

namespace e00::detail {
template<typename T, std::size_t N>
class CircularBuffer {
  static_assert(std::numeric_limits<uint8_t>::max() > N, "N is too big");

  Array<T, N> _internal;
  uint8_t _last_processed;
  uint8_t _next_free;

public:
  template<typename ...Args>
  T& Emplace(Args&& ...args) {
    if (_next_free >= N) _next_free = 0;
    _internal[_next_free++] = T(std::forward<Args>(args)...);
  }

  void Insert(T &&z) {
    if (_next_free >= N) _next_free = 0;
    _internal[_next_free++] = std::move(z);
  }

  void Insert(const T& z) {
    if (_next_free >= N) _next_free = 0;
    _internal[_next_free++] = std::move(z);
  }

  template<typename Func>
  void ForEach(Func &&func) {
    while (_last_processed != _next_free) {
      func(_internal[_last_processed]);
      _last_processed++;
    }

    _last_processed = _next_free = 0;
  }
};
}// namespace e00::detail
