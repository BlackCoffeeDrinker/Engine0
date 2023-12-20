#include "BoxedToLuaConverter.hpp"
#include "UserDataHolder.hpp"

#include <Engine/Scripting/BoxedCast.hpp>

#include <string_view>

using namespace e00::scripting;

int boxed_to_lua(lua_State *L, const BoxedValue &boxed_rv) {
  if (boxed_rv.is_void()) {
    return 0;
  }

  bool matched =//
    try_cast<bool>(boxed_rv, [&L](bool a) { lua_pushboolean(L, (a ? 1 : 0)); }) ||//
    try_cast<int>(boxed_rv, [&L](int a) { lua_pushinteger(L, a); }) ||//
    try_cast<long>(boxed_rv, [&L](long a) { lua_pushinteger(L, a); }) ||//
    try_cast<float>(boxed_rv, [&L](float a) { lua_pushnumber(L, a); }) ||//
    try_cast<std::string>(boxed_rv, [&L](const std::string &str) { lua_pushlstring(L, str.c_str(), str.size()); }) ||//
    try_cast<std::string_view>(boxed_rv, [&L](const std::string_view &str) { lua_pushlstring(L, str.data(), str.size()); });//

  if (!matched) {
    if (boxed_rv.is_class() || boxed_rv.is_pointer()) {
      // We need to send a class-like thing to Lua, we'll put it as user data
      auto **pudh = (lua::UserDataHolder **)lua_newuserdata(L, sizeof(lua::UserDataHolder *));
      if (!pudh) return 0;

      *pudh = new lua::UserDataHolder(boxed_rv);
      luaL_setmetatable(L, lua::UserDataHolder::MetaTableName);

      matched = true;
    }
  }

  return matched ? 1 : 0;
}
