#pragma once

#include <type_traits>

namespace e00::scripting::detail {

template<typename... Param>
struct Function_Params {
};

template<typename Ret, typename Params, bool IsNoExcept = false, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
struct FunctionSignature {
  using Param_Types = Params;
  using Return_Type = Ret;
  constexpr static const bool is_object = IsObject;
  constexpr static const bool is_member_object = IsMemberObject;
  constexpr static const bool is_noexcept = IsNoExcept;
  template<typename T>
  constexpr FunctionSignature(T &&) noexcept {}
  constexpr FunctionSignature() noexcept = default;
};

// Free functions

template<typename Ret, typename... Param>
FunctionSignature(Ret (*f)(Param...)) -> FunctionSignature<Ret, Function_Params<Param...>>;

template<typename Ret, typename... Param>
FunctionSignature(Ret (*f)(Param...) noexcept) -> FunctionSignature<Ret, Function_Params<Param...>, true>;

// no reference specifier

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile) -> FunctionSignature<Ret, Function_Params<volatile Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile noexcept) -> FunctionSignature<Ret, Function_Params<volatile Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const) -> FunctionSignature<Ret, Function_Params<volatile const Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const noexcept) -> FunctionSignature<Ret, Function_Params<volatile const Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...)) -> FunctionSignature<Ret, Function_Params<Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) noexcept) -> FunctionSignature<Ret, Function_Params<Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const) -> FunctionSignature<Ret, Function_Params<const Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const noexcept) -> FunctionSignature<Ret, Function_Params<const Class &, Param...>, true, true>;

// & reference specifier

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile &) -> FunctionSignature<Ret, Function_Params<volatile Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile &noexcept) -> FunctionSignature<Ret, Function_Params<volatile Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const &) -> FunctionSignature<Ret, Function_Params<volatile const Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const &noexcept) -> FunctionSignature<Ret, Function_Params<volatile const Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) &) -> FunctionSignature<Ret, Function_Params<Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) &noexcept) -> FunctionSignature<Ret, Function_Params<Class &, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const &) -> FunctionSignature<Ret, Function_Params<const Class &, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const &noexcept) -> FunctionSignature<Ret, Function_Params<const Class &, Param...>, true, true>;

// && reference specifier

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile &&) -> FunctionSignature<Ret, Function_Params<volatile Class &&, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile &&noexcept) -> FunctionSignature<Ret, Function_Params<volatile Class &&, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const &&) -> FunctionSignature<Ret, Function_Params<volatile const Class &&, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) volatile const &&noexcept) -> FunctionSignature<Ret, Function_Params<volatile const Class &&, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) &&) -> FunctionSignature<Ret, Function_Params<Class &&, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) &&noexcept) -> FunctionSignature<Ret, Function_Params<Class &&, Param...>, true, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const &&) -> FunctionSignature<Ret, Function_Params<const Class &&, Param...>, false, true>;

template<typename Ret, typename Class, typename... Param>
FunctionSignature(Ret (Class::*f)(Param...) const &&noexcept) -> FunctionSignature<Ret, Function_Params<const Class &&, Param...>, true, true>;

template<typename Ret, typename Class>
FunctionSignature(Ret(Class::*f)) -> FunctionSignature<Ret, Function_Params<Class &>, true, true, true>;

template<typename Func>
FunctionSignature(Func &&) -> FunctionSignature<
  typename decltype(FunctionSignature{ &std::decay_t<Func>::operator() })::Return_Type,
  typename decltype(FunctionSignature{ &std::decay_t<Func>::operator() })::Param_Types,
  decltype(FunctionSignature{ &std::decay_t<Func>::operator() })::is_noexcept,
  false,
  false,
  true>;

}// namespace e00::scripting::detail
