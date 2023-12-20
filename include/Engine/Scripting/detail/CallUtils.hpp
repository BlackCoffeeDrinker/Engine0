#pragma once

#include <memory>
#include <utility>

#include "../BoxedValue.hpp"
#include "../FunctionParams.hpp"
#include "HandleReturn.hpp"
#include "CastHelpers.hpp"

namespace e00::scripting::detail {
template<typename Callable, typename Ret, typename... Params, size_t... I>
Ret call_func1(Ret (*)(Params...),
  std::index_sequence<I...>,
  const Callable &f,
  [[maybe_unused]] const FunctionParams &params) {
  return f(CastHelper<Params>::cast(params[I])...);
}

/// Used by Proxy_Function_Impl to perform typesafe execution of a function.
/// The function attempts to unbox each parameter to the expected contained_type.
/// if any unboxing fails the execution of the function fails and
/// the bad_boxed_cast is passed up to the caller.
template<typename Callable, typename Ret, typename... Params>
BoxedValue call_func(Ret (*sig)(Params...), const Callable &f, const FunctionParams &params) {
  if constexpr (std::is_same_v<Ret, void>) {
    call_func1(sig, std::index_sequence_for<Params...>{}, f, params);
    return Handle_Return<void>::handle();
  } else {
    return Handle_Return<Ret>::handle(call_func1(sig, std::index_sequence_for<Params...>{}, f, params));
  }
}

}// namespace e00::scripting::detail
