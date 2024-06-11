#include "UserDataHolder.hpp"
#include "LuaScriptEngine.hpp"
#include "LuaToNativeTrampoline.hpp"
#include "BoxedToLuaConverter.hpp"

namespace e00::scripting::lua {
UserDataHolder::UserDataHolder() : _magic(MAGIC) {
}

UserDataHolder::UserDataHolder(BoxedValue value)
  : _magic(MAGIC), _value(std::move(value)) {
}

UserDataHolder::UserDataHolder(BoxedValue &&value)
  : _magic(MAGIC), _value(std::move(value)) {}

int UserDataHolder::LuaGc(lua_State *L) {
  const auto data_holder = static_cast<UserDataHolder **>(luaL_checkudata(L, 1, UserDataHolder::MetaTableName));
  if (data_holder && *data_holder) {
    delete *data_holder;
    *data_holder = nullptr;
  }

  return 0;
}
int UserDataHolder::LuaToString(lua_State *L) {
  const auto data_holder = static_cast<UserDataHolder **>(luaL_checkudata(L, 1, UserDataHolder::MetaTableName));
  if (data_holder && *data_holder) {
    lua_pushstring(L, "ScriptEngine_Data");
    return 1;
  }
  return 0;
}

int UserDataHolder::LuaIndex(lua_State *L) {
  auto ctx = static_cast<lua::LuaScriptEngine *>(lua_touserdata(L, lua_upvalueindex(1)));
  const auto data_holder = static_cast<UserDataHolder **>(luaL_checkudata(L, 1, UserDataHolder::MetaTableName));
  const auto methodName = lua_tostring(L, 2);

  if (const auto &fn = ctx->get_method_for_type((*data_holder)->_value.get_type_info(), methodName)) {
    // We want to call methodName on data_holder's object in ctx's context
    const auto values = extract_lua_arguments_for_proxyfunction(L, fn);
    return boxed_to_lua(L, fn->call(FunctionParams(values)));
  }

  return 0;
}

}// namespace e00::scripting::lua
