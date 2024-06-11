#include "NamedFunction.hpp"

#include <iostream>
#include <utility>

#include "BoxedToLuaConverter.hpp"
#include "LuaToBoxedConverter.hpp"

namespace e00::scripting::lua {

NamedFunction::NamedFunction(lua_State *L, std::string lua_fn_name, TypeInfo return_fn)
  : LuaProxyFunction(std::move(lua_fn_name), L, {}, return_fn, VARIABLE_ARGUMENTS) {
}

NamedFunction::~NamedFunction() = default;

BoxedValue NamedFunction::do_call(const FunctionParams &params) const {
  // Get a global of the function name
  lua_getglobal(_l, _fn_name.c_str());

  // This should already be true, but we never know if something went sideways at runtime
  if (!lua_isfunction(_l, -1)) {
    return {};
  }

  return call_lua_on_stack(params);
}
}// namespace e00::scripting::lua
