#pragma once

namespace e00 {

/**
 * ActionInstance is an action that was triggered at a point in time
 */
struct ActionInstance {
  Action action;
  GameClock::time_point when;
  constexpr ActionInstance() = default;
  constexpr ActionInstance(Action action, GameClock::time_point when) : action(action), when(when) {}
};
}