#pragma once

#include "Action.hpp"
#include "GameClock.hpp"

namespace e00 {
/**
 * ActionInstance is an action that was triggered at a point in time
 */
struct ActionInstance {
  Action action;

  GameClock::time_point when;

  constexpr ActionInstance() = default;

  constexpr ActionInstance(Action action, GameClock::time_point when) : action(action), when(when) {}

  bool operator<(const ActionInstance &rhs) const noexcept { return when < rhs.when; }
};
}// namespace e00
