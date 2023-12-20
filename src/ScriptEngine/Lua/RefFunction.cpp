#include "RefFunction.hpp"

namespace e00::scripting::lua {
RefFunction::RefFunction(lua_State *L, int ref_, TypeInfo return_type_hint)
  : LuaProxyFunction("<lambda>", L, {}, return_type_hint, VARIABLE_ARGUMENTS),
    _ref(ref_) {
}

RefFunction::RefFunction(std::string fn_name, lua_State *L, int ref_, TypeInfo return_type_hint)
  : LuaProxyFunction(std::forward<std::string>(fn_name), L, {}, return_type_hint, VARIABLE_ARGUMENTS),
    _ref(ref_) {
}

RefFunction::~RefFunction() {
  luaL_unref(_l, LUA_REGISTRYINDEX, _ref);
}
BoxedValue RefFunction::do_call(const FunctionParams &params) const {
  // Get our ref
  lua_rawgeti(_l, LUA_REGISTRYINDEX, _ref);

  return call_lua_on_stack(params);
}

}// namespace e00::scripting::lua
