#pragma once

namespace e00 {
using type_t = const void *;

/*
 The platforms we build for probably don't support RTTI
 (final executable would be too big for memory)

 So use this instead
 */
template<typename T>
type_t type_id() {
  static type_t a;
  return &a;
}
}// namespace e00
