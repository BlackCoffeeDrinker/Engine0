#pragma once

#include <functional>
#include <memory>
#include <string_view>

#include <Engine/Platform/Stream.hpp>

namespace e00::impl {
struct IniParser {
  struct Item {
    std::string_view category;
    std::string_view key;
    std::string_view value;
  };

  static error_code Parse(Stream &stream, const std::function<error_code(const Item &)> &);
};
}// namespace e00::impl
