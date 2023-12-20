#pragma once

#include <string>

#include "LuaProxyFunction.hpp"
#include "Lua.hpp"

namespace e00::scripting::lua {
class NamedFunction final : public LuaProxyFunction {

protected:
  BoxedValue do_call(const FunctionParams &params) const override;

public:
  NamedFunction(lua_State *L, std::string lua_fn_name, TypeInfo return_fn);

  ~NamedFunction() override;
};
}// namespace e00::scripting::lua
