#pragma once

namespace experimental {
/**
 * Represent a source location, useful for logging
 */
struct source_location {
  static constexpr source_location current(
    uint_least32_t line = __builtin_LINE(),
    const char *const file = __builtin_FILE()) noexcept {
    source_location ret;
    ret._line = line;
    ret._file = file;
    return ret;
  }

  constexpr source_location() noexcept = default;

  [[nodiscard]] constexpr uint_least32_t line() const noexcept { return _line; }
  [[nodiscard]] constexpr const char *file_name() const noexcept { return _file; }

private:
  uint_least32_t _line{};
  const char *_file = "";
};
}// namespace experimental

namespace e00 {
using source_location = experimental::source_location;
}
