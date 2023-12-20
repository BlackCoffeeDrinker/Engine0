#pragma once

#include <Engine/Scripting/BoxedValue.hpp>

#include "Lua.hpp"

/**
 * Convert to a specific contained_type
 *
 * @param L
 * @param n
 * @param info
 * @return
 */
e00::scripting::BoxedValue lua_to_boxed_value(lua_State *L, int n, const e00::scripting::TypeInfo &info);

/**
 * Convert to closest matching contained_type
 *
 * @param L
 * @param n
 * @return
 */
e00::scripting::BoxedValue lua_to_boxed_value_guess(lua_State *L, int n);
