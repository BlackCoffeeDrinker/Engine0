#include "LuaToNativeTrampoline.hpp"
#include "LuaToBoxedConverter.hpp"
#include "BoxedToLuaConverter.hpp"
#include "TrampolineData.hpp"

using namespace e00::scripting;

extern "C" int lua_trampoline(lua_State *L) {
  auto ctx = static_cast<lua::TrampolineData *>(lua_touserdata(L, lua_upvalueindex(1)));

  if (!ctx) {
    // oops!
    return 0;
  }

  const auto values = extract_lua_arguments_for_proxyfunction(L, ctx->fn());
  return boxed_to_lua(L, ctx->fn()->call(FunctionParams(values)));
}

std::vector<BoxedValue> extract_lua_arguments_for_proxyfunction(lua_State *L, const std::unique_ptr<e00::scripting::ProxyFunction> &func) {
  const auto args_count = lua_gettop(L);

  // TODO: Varargs support
  if (/*!func->is_varargs() && */ static_cast<size_t>(args_count) < func->parameter_count()) {
    // TODO: Figure out what to do
    return {};
  }

  std::vector<BoxedValue> values;
  if (args_count > 0) {
    // Reserve for the number of arguments the function wants
    values.reserve(func->parameter_count());

    // Loop over all the arguments provided by Lua and convert them
    for (size_t i = 0; i < func->parameter_count(); i++) {
      values.emplace_back(lua_to_boxed_value(L, static_cast<int>(i + 1), func->parameter(i)));
    }
  }

  return values;
}
