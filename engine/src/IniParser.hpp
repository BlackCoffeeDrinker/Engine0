#pragma once

#include <memory>
#include <system_error>
#include <string_view>
#include <functional>

#include <../include/Engine/Platform/Stream.hpp>

namespace e00::impl {
struct IniParser {
  struct Item {
    std::string_view category;
    std::string_view key;
    std::string_view value;
  };

  static std::error_code Parse(Stream& stream, const std::function<std::error_code(const Item&)>&);
};
}