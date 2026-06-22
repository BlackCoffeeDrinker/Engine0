
#pragma once

namespace e00 {
struct LoadOption {
  type_t optionTypeid = {};
  LoadOption() = default;
  virtual ~LoadOption() = default;
  constexpr explicit LoadOption(type_t t) : optionTypeid(t) {}
};

template<typename T>
struct LoadOptionT : LoadOption {
  constexpr LoadOptionT() : LoadOption(type_id<T>()) {}
};

// Default known load options
struct DiscardPalette : LoadOptionT<DiscardPalette> {};

}
