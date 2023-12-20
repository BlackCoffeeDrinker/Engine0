#pragma once

#include <system_error>

namespace e00::scripting::lua {
enum class Error {
  RuntimeError = 1,
  SyntaxError,
  OutOfMemory,
  GenericError
};

std::error_code make_error_code(Error);
std::error_code lua_ret_to_error_code(int lua_rc);
}// namespace e00::scripting::lua

namespace std {
template<>
struct is_error_code_enum<e00::scripting::lua::Error> : true_type {};
}// namespace std
