#pragma once

#include "BoxedValue.hpp"
#include "detail/CastHelpers.hpp"

namespace e00::scripting {

template<typename Type>
Type cast(const BoxedValue &bv) {
  return detail::CastHelper<Type>::cast(std::forward<decltype(bv)>(bv));
}

template<typename Type, typename UnBoxedFn>
bool try_cast(const BoxedValue &bv, UnBoxedFn &&fn) {
  if (bv.get_type_info().bare_equal_type_info(user_type<Type>()) || bv.get_type_info().bare_equal_type_info(user_type<Type *>())) {
    fn(detail::CastHelper<Type>::cast(std::forward<decltype(bv)>(bv)));
    return true;
  }
  return false;
}
}// namespace e00::scripting
