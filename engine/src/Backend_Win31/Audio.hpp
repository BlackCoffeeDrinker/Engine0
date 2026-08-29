#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

#include <cstdint>
#include <span>

namespace win31 {

// Minimal waveOut double-buffer PCM player for Win32s.
void InitAudio();
void ShutdownAudio();
void PumpAudio(); // reclaim completed buffers; call once per frame
void QueuePcmMono16(std::span<const int16_t> samples, uint32_t sampleRateHz);

} // namespace win31
