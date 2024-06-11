#pragma once

#include "Utils.hpp"
#include "../ProxyFunction.hpp"
#include "FunctionSignature.hpp"
#include "CallUtils.hpp"
#include "AttributeAccess.hpp"

namespace e00::scripting::detail {
template<typename Obj, typename Param1, typename... Rest>
Param1 get_first_param(Function_Params<Param1, Rest...>, Obj &&obj) {
  return static_cast<Param1>(std::forward<Obj>(obj));
}

template<typename Ret, typename... Params>
std::vector<TypeInfo> build_param_type_list(Ret (*)(Params...)) {
  return { user_type<Params>()... };
}

template<typename Ret, typename... Params>
TypeInfo build_return_type_list(Ret (*)(Params...)) {
  return user_type<Ret>();
}

/**
 *
 * @tparam Func
 * @tparam Callable
 */
template<typename Func, typename Callable>
struct NativeFunctionMemberT final : ProxyFunction {
  explicit NativeFunctionMemberT(Callable f, size_t argcnt)
    : ProxyFunction(
      build_param_type_list(static_cast<Func *>(nullptr)),
      build_return_type_list(static_cast<Func *>(nullptr)),
      argcnt),
      _f(std::move(f)) {
  }

  bool is_member() const noexcept override {
    return true;
  }

protected:
  BoxedValue do_call(const FunctionParams &params) const override {
    return call_func(static_cast<Func *>(nullptr), _f, params);
  }

private:
  Callable _f;
};

/**
 *
 * @tparam Func
 * @tparam Callable
 */
template<typename Func, typename Callable>
struct NativeFunctionT final : ProxyFunction {
  explicit NativeFunctionT(Callable f, size_t argcnt)
    : ProxyFunction(
      build_param_type_list(static_cast<Func *>(nullptr)),
      build_return_type_list(static_cast<Func *>(nullptr)),
      argcnt),
      _f(std::move(f)) {
  }

protected:
  BoxedValue do_call(const FunctionParams &params) const override {
    return call_func(static_cast<Func *>(nullptr), _f, params);
  }

private:
  Callable _f;
};

template<typename Func, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject, bool Is_Object, typename Ret, typename... Param>
std::unique_ptr<ProxyFunction> make_callable_impl(Func &&func, FunctionSignature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, Is_Object>) {
  if constexpr (Is_MemberObject) {
    // we now that the Param pack will have only one element, so we are safe expanding it here
    return make_unique_base<ProxyFunction, Attribute_Access<Ret, std::decay_t<Param>...>>(std::forward<Func>(func));
  } else if constexpr (Is_Member) {
    auto call = [func = std::forward<Func>(func)](auto &&obj, auto &&...param) /*noexcept(Is_Noexcept)*/ -> decltype(auto) {
      return ((get_first_param(Function_Params<Param...>{}, obj).*func)(std::forward<decltype(param)>(param)...));
    };

    return make_unique_base<ProxyFunction, NativeFunctionMemberT<Ret(Param...), decltype(call)>>(std::move(call), sizeof...(Param));
  } else {
    return make_unique_base<ProxyFunction, NativeFunctionT<Ret(Param...), std::decay_t<Func>>>(std::forward<Func>(func), sizeof...(Param));
  }
}

// this version peels off the function object itself from the function signature, when used
// on a callable object
template<typename Func, typename Ret, typename Object, typename... Param, bool Is_Noexcept>
std::unique_ptr<ProxyFunction> make_function_t(Func &&func, FunctionSignature<Ret, Function_Params<Object, Param...>, Is_Noexcept, false, false, true>) {
  return make_callable_impl(std::forward<Func>(func), FunctionSignature<Ret, Function_Params<Param...>, Is_Noexcept, false, false, true>{});
}

template<typename Func, typename Ret, typename... Param, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject>
std::unique_ptr<ProxyFunction> make_function_t(Func &&func, FunctionSignature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, false> fs) {
  return make_callable_impl(std::forward<Func>(func), fs);
}
}// namespace e00::scripting::detail
