#pragma once

namespace experimental {
/**
 * Represent a source location, useful for logging
 */
struct source_location {
  static constexpr source_location current(
    uint_least32_t line = __builtin_LINE(),
#ifdef DJGPP
    const uint_least32_t col = 0,
#else
    const uint_least32_t col = __builtin_COLUMN(),
#endif
    const char *const file = __builtin_FILE(),
    const char *const func = __builtin_FUNCTION()) noexcept {
    source_location ret;
    ret._line = line;
    ret._col = col;
    ret._file = file;
    ret._func = func;
    return ret;
  }

  constexpr source_location() noexcept = default;

  [[nodiscard]] constexpr uint_least32_t line() const noexcept { return _line; }
  [[nodiscard]] constexpr uint_least32_t column() const noexcept { return _col; }
  [[nodiscard]] constexpr const char *file_name() const noexcept { return _file; }
  [[nodiscard]] constexpr const char *function_name() const noexcept { return _func; }

private:
  uint_least32_t _line{};
  uint_least32_t _col{};
  const char *_file = "";
  const char *_func = "";
};
}// namespace experimental

namespace e00 {
using source_location = experimental::source_location;
}
