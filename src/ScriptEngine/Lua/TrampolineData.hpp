#pragma once

#include <memory>
#include <string>
#include <Engine/Scripting/ProxyFunction.hpp>

#include "Lua.hpp"

namespace e00::scripting::lua {
class LuaScriptEngine;

class TrampolineData {
  LuaScriptEngine *_engine;
  std::unique_ptr<ProxyFunction> _fn;
  std::string _lua_name;

public:
  TrampolineData(LuaScriptEngine *engine, std::unique_ptr<ProxyFunction> &&fn, std::string name)
    : _engine(engine), _fn(std::move(fn)), _lua_name(std::move(name)) {}

  [[nodiscard]] const std::unique_ptr<ProxyFunction> &fn() const { return _fn; }
};
}// namespace e00::scripting::lua
