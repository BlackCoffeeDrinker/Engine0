#pragma once

#include <Engine/Detail/ErrorCode.hpp>

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

error_code make_error_code(EngineErrorCode);
}// namespace e00::impl

namespace e00 {
template<>
struct is_error_code_enum<impl::EngineErrorCode> : std::true_type {};
}// namespace e00
