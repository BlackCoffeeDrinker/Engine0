#pragma once

#include <system_error>
#include <map>

#include <Engine.hpp>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace {
const std::string lua_engine_name = "Lua";
}

namespace e00::scripting::lua {
class LuaScriptEngine : public ScriptEngine {
  lua_State *_state;

  std::map<type_id_no_rtti, std::map<std::string, std::unique_ptr<scripting::ProxyFunction>>> _methods;
  std::unique_ptr<scripting::ProxyFunction> _bad_method;

public:
  LuaScriptEngine();

  ~LuaScriptEngine() override;

  const std::string &engine_name() const override { return lua_engine_name; }

protected:
  bool valid_fn_name(const std::string &fn_name) override;
  void add_function(const std::string &fn_name, std::unique_ptr<scripting::ProxyFunction> &&fn) override;
  void add_variable(const std::string &var_name, scripting::BoxedValue val) override;
  void add_type(const TypeInfo &type) override;

public:
  void log_from_lua(int level, const std::string_view &str);

  std::error_code parse(const std::string &code) override;

  std::unique_ptr<scripting::ProxyFunction> get_function(const std::string &fn_name, scripting::TypeInfo preferred_return_type) override;
  std::error_code parse(const std::unique_ptr<e00::Stream> &stream) override;
  const std::unique_ptr<scripting::ProxyFunction> &get_method_for_type(const TypeInfo &type, const std::string &method_name) const;
};
}// namespace e00::scripting::lua
