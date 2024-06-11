#include "LuaProxyFunction.hpp"

#include <iostream>

#include "BoxedToLuaConverter.hpp"
#include "LuaToBoxedConverter.hpp"

namespace e00::scripting::lua {
BoxedValue LuaProxyFunction::call_lua_on_stack(const FunctionParams &params) const {
  // Keep a record of how the stack is
  int stack_size = lua_gettop(_l);

  // Push our arguments into the stack
  for (const auto &p : params) {
    if (boxed_to_lua(_l, p) != 1) {
      std::cerr << "Error calling " << _fn_name << "\n"
                << "Don't know how to push arg\n";
      return BoxedValue();
    }
  }

  // Call the lua function and accept up to 1 return argument
  if (lua_pcall(_l, static_cast<int>(params.size()), 1 /*LUA_MULTRET*/, 0)) {
    std::cerr << "Error calling " << _fn_name << "\n"
              << lua_error(_l) << "\n";
    return BoxedValue();
  }

  // Count number of returns (the function was already pushed, so remove that one)
  int num_returns = lua_gettop(_l) - (stack_size - 1);
  if (num_returns == 0) {
    return BoxedValue();
  }

  auto r = lua_to_boxed_value_guess(_l, -1);
  lua_pop(_l, 1);
  return r;
}
}// namespace e00::scripting::lua
