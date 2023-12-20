#pragma once

#include <memory>
#include <tuple>

namespace e00::scripting::detail {
template<typename B, typename D, typename... Arg>
std::unique_ptr<B> make_unique_base(Arg &&...arg) {
  D *new_obj = new D(std::forward<Arg>(arg)...);
  return std::unique_ptr<B>(static_cast<B *>(new_obj));
}
}// namespace e00::scripting::detail
