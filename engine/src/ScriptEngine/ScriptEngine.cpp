#include "Lua/LuaScriptEngine.hpp"
#include <Engine.hpp>

namespace e00 {
ScriptEngine::ScriptEngine() {
}

ScriptEngine::~ScriptEngine() {
}

std::unique_ptr<ScriptEngine> ScriptEngine::Create() {
  return std::unique_ptr<ScriptEngine>(new scripting::lua::LuaScriptEngine());
}

error_code ScriptEngine::parse(const std::unique_ptr<Stream> &stream) {
  std::string script;
  if (stream) {
    script.resize(stream->Size());
    if (auto stream_ec = stream->Read(stream->Size(), script.data())) {
      return stream_ec;
    }

    return parse(script);
  }

  return {};
}

namespace scripting {
const TypeInfo ProxyFunction::_end = TypeInfo();
}

}// namespace e00
