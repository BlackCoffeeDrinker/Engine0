#pragma once

#include <chrono>

namespace e00 {
struct GameClock {
  typedef std::chrono::milliseconds          duration;
  typedef duration::rep                      rep;
  typedef duration::period                   period;
  typedef std::chrono::time_point<GameClock> time_point;
  static const bool                          is_steady = false;
};
}