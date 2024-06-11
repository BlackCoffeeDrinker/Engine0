#pragma once

#include <string>
#include <Engine/Scripting/ProxyFunction.hpp>

#include "Lua.hpp"

namespace e00::scripting::lua {
/**
 * Base class for a LUA proxy function
 */
class LuaProxyFunction : public ProxyFunction {
protected:
  const std::string _fn_name;
  lua_State *_l;

  LuaProxyFunction(std::string fn_name, lua_State *L, std::vector<TypeInfo> types, TypeInfo return_type, size_t args_cnt)
    : ProxyFunction(std::forward<decltype(types)>(types), return_type, args_cnt),
      _fn_name(std::move(fn_name)), _l(L) {}

  BoxedValue call_lua_on_stack(const FunctionParams &params) const;

public:
  ~LuaProxyFunction() override = default;
};
}// namespace e00::scripting::lua
