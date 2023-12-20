#pragma once

#include <vector>
#include <Engine/Scripting/ProxyFunction.hpp>
#include "Lua.hpp"

extern "C" int lua_trampoline(lua_State *L);

std::vector<e00::scripting::BoxedValue> extract_lua_arguments_for_proxyfunction(lua_State *L, const std::unique_ptr<e00::scripting::ProxyFunction> &func);
