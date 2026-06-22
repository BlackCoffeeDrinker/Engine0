#pragma once
#include "Engine/Detail/TypeId.hpp"
#include <string_view>

namespace e00 {
using ResourceId = uint32_t;

constexpr ResourceId HashName(std::string_view str) noexcept {
  ResourceId hash = 2166136261U;
  for (const auto c: str) {
    // Optional: Turn characters uppercase to make asset names case-insensitive
    const char up = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    hash ^= static_cast<ResourceId>(up);
    hash *= 16777619U;
  }
  return hash;
}

// User-defined literal for ultra-clean code syntax: e00_id
constexpr ResourceId operator""_id(const char *str, size_t len) noexcept {
  return HashName(std::string_view(str, len));
}

struct Resource {
  Resource() = default;// Resources need to be default-constructible
  virtual ~Resource() = default;

  [[nodiscard]] virtual type_t Type() const = 0;

  template<typename T>
  [[nodiscard]] bool Is() const { return Type() == type_id<T>(); }

  template<typename T>
  T &As() {
    if (Is<T>()) return static_cast<T &>(*this);
    std::abort();
  }
};
}// namespace e00
