#pragma once

#include <cstdint>
#include <Engine/Scripting/BoxedValue.hpp>

#include "Lua.hpp"

namespace e00::scripting::lua {
class UserDataHolder {
  constexpr static uint32_t MAGIC = 0xADC0DEAD;

  uint32_t _magic;// < LEAVE THIS AS THE FIRST ELEMENT
  BoxedValue _value;

public:
  constexpr static const char *MetaTableName = "SE_UserDataHolder";

  static int LuaGc(lua_State *);

  static int LuaToString(lua_State *);

  static int LuaIndex(lua_State *);

  UserDataHolder();

  UserDataHolder(BoxedValue &&value);

  UserDataHolder(BoxedValue value);

  ~UserDataHolder() = default;

  [[nodiscard]] bool valid() const noexcept { return _magic == MAGIC; }

  [[nodiscard]] BoxedValue BoxedData() const { return _value; }
};

}// namespace e00::scripting::lua
