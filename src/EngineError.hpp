#pragma once

#include <system_error>

namespace e00::impl {
enum class EngineErrorCode {
  not_configured = 1,
  invalid_argument,
  resource_not_found,
  bad_configuration_file,
  level_is_not_valid,
  error_building_level,
  out_of_memory,
};

std::error_code make_error_code(EngineErrorCode);
}// namespace e00::impl

namespace std {
template<>
struct is_error_code_enum<e00::impl::EngineErrorCode> : true_type {};
}// namespace std
