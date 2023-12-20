#pragma once

#include "Engine/Scripting/BoxedValue.hpp"
#include "Lua.hpp"

int boxed_to_lua(lua_State *L, const e00::scripting::BoxedValue &boxed_rv);
