#pragma once

#include "BoxedValue.hpp"

namespace e00::scripting {

template<typename Type>
Type cast(const BoxedValue &bv) {
  return bv.template cast<Type>();
}

template<typename Type, typename UnBoxedFn>
bool try_cast(const BoxedValue &bv, UnBoxedFn &&fn) {
  return bv.try_cast<Type>(std::forward<UnBoxedFn>(fn));
}
}// namespace e00::scripting
