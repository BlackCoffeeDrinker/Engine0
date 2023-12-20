#include <Engine.hpp>
#include "Lua/LuaScriptEngine.hpp"

namespace e00 {
ScriptEngine::ScriptEngine() {
}

ScriptEngine::~ScriptEngine() {
}

std::unique_ptr<ScriptEngine> ScriptEngine::Create() {
  return std::unique_ptr<ScriptEngine>(new scripting::lua::LuaScriptEngine());
}

std::error_code ScriptEngine::parse(const std::unique_ptr<e00::Stream> &stream) {
  std::string script;
  if (stream) {
    script.reserve(stream->stream_size());
    if (auto stream_ec = stream->read(stream->stream_size(), script.data())) {
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
