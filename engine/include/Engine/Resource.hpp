#pragma once
#include "Engine/Detail/TypeId.hpp"
#include <string_view>

namespace e00 {
struct Resource {
  Resource() = default;// Resources need to be default-constructible
  virtual ~Resource() = default;

  [[nodiscard]] virtual type_t Type() const = 0;
  [[nodiscard]] virtual size_t SizeUsage() = 0;

  template<typename T>
  [[nodiscard]] bool Is() const { return Type() == type_id<T>(); }

  template<typename T>
  T &As() {
    if (Is<T>()) return static_cast<T &>(*this);
    std::abort();
  }
};
}// namespace e00
