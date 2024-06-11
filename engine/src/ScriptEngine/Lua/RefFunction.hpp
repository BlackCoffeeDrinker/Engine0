#pragma once

#include <string_view>

#include "LuaProxyFunction.hpp"
#include "Lua.hpp"

namespace e00::scripting::lua {
class RefFunction final : public LuaProxyFunction {
  int _ref = 0;

public:
  RefFunction(lua_State *L, int ref_, TypeInfo return_type_hint = TypeInfo());

  RefFunction(std::string fn_name, lua_State *L, int ref_, TypeInfo return_type_hint = TypeInfo());

  ~RefFunction() override;

protected:
  BoxedValue do_call(const FunctionParams &params) const override;
};
}// namespace e00::scripting::lua
