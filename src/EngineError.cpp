#include "EngineError.hpp"

namespace {
struct EngineErrorCategory : std::error_category {
  [[nodiscard]] const char *name() const noexcept override { return "E00 Error"; }
  [[nodiscard]] std::string message(int ev) const override {
    switch (static_cast<e00::impl::EngineErrorCode>(ev)) {
      case e00::impl::EngineErrorCode::not_configured: return "not configured";
      case e00::impl::EngineErrorCode::invalid_argument: return "invalid argument";
      case e00::impl::EngineErrorCode::resource_not_found: return "specified resource was not found";
      case e00::impl::EngineErrorCode::bad_configuration_file: return "bad configuration file";
      case e00::impl::EngineErrorCode::level_is_not_valid: return "not a valid resource";
      case e00::impl::EngineErrorCode::error_building_level: return "unable to general level data";
      case e00::impl::EngineErrorCode::out_of_memory: return "out of memory";
      default: return "";
    }
  }
};

const EngineErrorCategory engine_err_category{};
}// namespace

namespace e00::impl {
std::error_code make_error_code(EngineErrorCode e) {
  return {static_cast<int>(e), engine_err_category};
}
}// namespace e00::impl
