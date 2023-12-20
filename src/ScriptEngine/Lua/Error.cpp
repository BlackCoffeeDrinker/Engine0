#include "Error.hpp"
#include "Lua.hpp"

namespace {
struct LuaErrorCategory : std::error_category {
  const char *name() const noexcept override;
  std::string message(int ev) const override;
};

const char *LuaErrorCategory::name() const noexcept {
  return "engine";
}

std::string LuaErrorCategory::message(int ev) const {
  switch (static_cast<e00::scripting::lua::Error>(ev)) {
    case e00::scripting::lua::Error::RuntimeError: break;
    case e00::scripting::lua::Error::SyntaxError: break;
    case e00::scripting::lua::Error::OutOfMemory: break;
    case e00::scripting::lua::Error::GenericError: break;
  }
  return {};
}

const LuaErrorCategory luaErrorCategory{};
}// namespace

namespace e00::scripting::lua {
std::error_code make_error_code(Error r) {
  return std::error_code(static_cast<int>(r), luaErrorCategory);
}

std::error_code lua_ret_to_error_code(int lua_rc) {
  switch (lua_rc) {
    case LUA_OK: return {};
    case LUA_ERRRUN: return make_error_code(Error::RuntimeError);
    case LUA_ERRSYNTAX: return make_error_code(Error::SyntaxError);
    case LUA_ERRMEM: return make_error_code(Error::OutOfMemory);
    case LUA_ERRERR: return make_error_code(Error::GenericError);
  }

  return {};
}
}// namespace e00::scripting::lua
