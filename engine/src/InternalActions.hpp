#pragma once

#include <Engine.hpp>

namespace e00::impl {
enum class EngineAction : int {
  Quit = 1
};

Action make_action(EngineAction e);
}// namespace e00::impl

namespace e00 {
template<>
struct IsActionTypeEnum<impl::EngineAction> : std::true_type {};

}// namespace e00
