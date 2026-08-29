#include "Platform.hpp"

#include <chrono>

namespace platform {

system_clock::time_point system_clock::now() noexcept {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  return time_point(duration(static_cast<rep>(millis)));
}

void SetMenu(e00::Engine & /*engine*/, std::span<const MenuItem> /*items*/) {
  // DOS has no native menu bar.
}

void QueueAudioSamples(std::span<const int16_t> /*pcmMono16*/, uint32_t /*sampleRateHz*/) {
  // DOS audio backend not wired through this API yet.
}

} // namespace platform
