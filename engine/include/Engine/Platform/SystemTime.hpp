#pragma once

#include <chrono>
#include <cstdint>

namespace platform {

/**
 * A minimal Clock (satisfying the standard Clock requirements: `duration`,
 * `rep`, `period`, `time_point`, `is_steady`, `now()`), implemented by each
 * backend individually. Shared core code should use
 * platform::system_clock::now() wherever it previously used
 * std::chrono::steady_clock::now()/std::chrono::system_clock::now().
 *
 * `now()` is deliberately declared here but defined per-backend (not inline)
 * because calling the C++ runtime's own
 * std::chrono::steady_clock::now()/system_clock::now() directly from shared
 * core code is NOT safe on every target: on the Win31/Win32s backend, the
 * statically-linked libstdc++ implementation of those clocks pulls in SJLJ
 * exception-frame bookkeeping and a gettimeofday() path that crashes on real
 * Win32s hardware. Routing all timing through a single backend-provided
 * now() keeps the shared core portable without depending on std::chrono's
 * clock implementations at all.
 */
struct system_clock {
  using duration = std::chrono::milliseconds;
  using rep = duration::rep;
  using period = duration::period;
  using time_point = std::chrono::time_point<system_clock>;
  static constexpr bool is_steady = true;

  static time_point now() noexcept;
};

}// namespace platform
