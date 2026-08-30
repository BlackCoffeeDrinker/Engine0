#pragma once

#include <Engine/Detail/ErrorCode.hpp>

namespace e00::scripting::lua {
enum class Error {
  RuntimeError = 1,
  SyntaxError,
  OutOfMemory,
  GenericError
};

error_code make_error_code(Error);
error_code lua_ret_to_error_code(int lua_rc);
}// namespace e00::scripting::lua

namespace e00 {
template<>
struct is_error_code_enum<scripting::lua::Error> : std::true_type {};
}// namespace std
