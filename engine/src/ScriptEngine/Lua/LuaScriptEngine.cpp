#include "LuaScriptEngine.hpp"

#include <sstream>

#include "TrampolineData.hpp"
#include "LuaToNativeTrampoline.hpp"
#include "LuaToBoxedConverter.hpp"
#include "NamedFunction.hpp"
#include "RefFunction.hpp"
#include "BoxedToLuaConverter.hpp"
#include "UserDataHolder.hpp"
#include "Error.hpp"

namespace {
void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
  // auto *engine = static_cast<e00::impl::scripting::lua::LuaScriptEngine *>(ud);
  (void)ud;
  (void)osize; /* not used */

  if (nsize == 0) {
    free(ptr);
    return nullptr;
  } else
    return realloc(ptr, nsize);
}


int lua_log(lua_State *L) {
  using namespace e00::scripting;
  lua_Integer level = lua_tointegerx(L, lua_upvalueindex(1), nullptr);
  auto *engine = static_cast<lua::LuaScriptEngine *>(lua_touserdata(L, lua_upvalueindex(2)));

  // make the string
  std::stringstream ss;
  const auto args_count = lua_gettop(L);

  for (int i = 0; i < args_count; i++) {
    auto boxed_value = lua_to_boxed_value_guess(L, i + 1);

    if (boxed_value.get_type_info().bare_equal_type_info(user_type<std::string>())) {
      ss << cast<std::string>(boxed_value);
    } else if (boxed_value.get_type_info().bare_equal_type_info(user_type<int>())) {
      ss << cast<int>(boxed_value);
    } else if (boxed_value.get_type_info().bare_equal_type_info(user_type<float>())) {
      ss << cast<float>(boxed_value);
    }

    // TODO: Support userdata
  }

  // is it level 0 (info) or 1 (error)
  engine->log_from_lua(level, ss.str());

  return 0;
}
}// namespace

namespace e00::scripting::lua {
LuaScriptEngine::LuaScriptEngine()
  : ScriptEngine(),
    _state(lua_newstate(&l_alloc, this)),
    _bad_method(nullptr) {
  // Make our genetic metatable for userdata
  // The if is technically not required as there's no chance we re-execute this
  if (luaL_newmetatable(_state, UserDataHolder::MetaTableName)) {
    lua_pushcfunction(_state, &UserDataHolder::LuaGc);
    lua_setfield(_state, -2, "__gc");

    lua_pushlightuserdata(_state, this);
    lua_pushcclosure(_state, &UserDataHolder::LuaIndex, 1);
    lua_setfield(_state, -2, "__index");

    lua_pushcfunction(_state, &UserDataHolder::LuaToString);
    lua_setfield(_state, -2, "__tostring");

    lua_pop(_state, 1);
  }

  // Push special log functions
  lua_pushinteger(_state, 0);
  lua_pushlightuserdata(_state, this);
  lua_pushcclosure(_state, &lua_log, 2);
  lua_setglobal(_state, "info");

  lua_pushinteger(_state, 1);
  lua_pushlightuserdata(_state, this);
  lua_pushcclosure(_state, &lua_log, 2);
  lua_setglobal(_state, "error");
}

LuaScriptEngine::~LuaScriptEngine() {
  lua_close(_state);
}

bool LuaScriptEngine::valid_fn_name(const std::string &fn_name) {
  // TODO: Validate name
  return !fn_name.empty();
}

void LuaScriptEngine::add_function(const std::string &fn_name, std::unique_ptr<ProxyFunction> &&fn) {
  if (fn->is_member()) {
    // This will be used for <something>:fn_name
    const auto type_id = fn->parameter(0);
    _methods[type_id.bare_id()].insert(std::make_pair(fn_name, std::move(fn)));
    return;
  }

  lua_pushlightuserdata(_state, new TrampolineData(this, std::move(fn), fn_name));
  lua_pushcclosure(_state, &lua_trampoline, 1);
  lua_setglobal(_state, fn_name.c_str());
}

void LuaScriptEngine::add_variable(const std::string &var_name, scripting::BoxedValue val) {
  auto ret = boxed_to_lua(_state, val);
  if (ret == 1) {
    lua_setglobal(_state, var_name.c_str());
  }
}

void LuaScriptEngine::add_type(const TypeInfo &type) {
  if (!type.is_class()) {
    return;
  }
}

std::error_code LuaScriptEngine::parse(const std::string &code) {
  struct ReaderState {
    const std::string_view &data;
    size_t read_pos;
  };

  lua_Reader reader = [](lua_State *, void *user_data, size_t *out_size) -> const char * {
    auto *state = static_cast<ReaderState *>(user_data);
    *out_size = 0;

    if (state->read_pos < state->data.size()) {
      *out_size = state->data.size();
      state->read_pos = state->data.size();
      return state->data.data();
    }

    return nullptr;
  };

  ReaderState state{ code, 0 };
  if (auto ec = lua_ret_to_error_code(lua_load(_state, reader, &state, "native_load", nullptr))) {
    return ec;
  }

  return lua_ret_to_error_code(lua_pcall(_state, 0, 0, 0));
}

std::error_code LuaScriptEngine::parse(const std::unique_ptr<e00::Stream> &stream) {
  struct ReaderState {
    char buffer[256];
    const std::unique_ptr<e00::Stream> &stream;
  };

  lua_Reader reader = [](lua_State *, void *ud, size_t *sz) -> const char * {
    auto *state = static_cast<ReaderState *>(ud);
    *sz = 0;

    if (const auto max_read = state->stream->max_read(); max_read > 0) {
      *sz = 255;
      if (*sz > max_read) {
        *sz = max_read;
      }

      state->stream->read(*sz, state->buffer);
      return state->buffer;
    }

    return nullptr;
  };

  ReaderState state{ {}, stream };

  if (auto ec = lua_ret_to_error_code(lua_load(_state, reader, &state, "parse(stream)", nullptr))) {
    return ec;
  }

  return lua_ret_to_error_code(lua_pcall(_state, 0, 0, 0));
}

std::unique_ptr<scripting::ProxyFunction> LuaScriptEngine::get_function(const std::string &fn_name, scripting::TypeInfo preferred_return_type) {
  // Get a global of the function name
  lua_getglobal(_state, fn_name.c_str());

  // This should already be true, but we never know if something went sideways at runtime
  if (!lua_isfunction(_state, -1)) {
    return nullptr;
  }

  // take a ref to it
  //return std::unique_ptr<scripting::ProxyFunction>(new RefFunction(fn_name, _state, luaL_ref(_state, LUA_REGISTRYINDEX), preferred_return_type));
  return std::make_unique<RefFunction>(fn_name, _state, luaL_ref(_state, LUA_REGISTRYINDEX), preferred_return_type);
}

const std::unique_ptr<scripting::ProxyFunction> &LuaScriptEngine::get_method_for_type(const TypeInfo &type, const std::string &method_name) const {
  if (const auto it = _methods.find(type.bare_id()); it != _methods.end()) {
    const auto mit = it->second.find(method_name);
    if (mit != it->second.end()) {
      return mit->second;
    }
  }

  return _bad_method;
}
void LuaScriptEngine::log_from_lua(int level, const std::string_view &str) {

}

}// namespace e00::scripting::lua
