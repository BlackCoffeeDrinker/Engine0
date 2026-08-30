#include "Audio.hpp"

#include <cstring>

namespace win31 {
namespace {

constexpr int kNumBuffers = 2;
constexpr DWORD kBufferBytes = 2048;// ~1024 mono int16 samples

struct AudioBuffer {
  WAVEHDR hdr{};
  char data[kBufferBytes]{};
  bool inFlight = false;
};

HWAVEOUT g_waveOut = nullptr;
uint32_t g_sampleRate = 0;
AudioBuffer g_buffers[kNumBuffers]{};
int g_nextBuffer = 0;
bool g_ready = false;

bool EnsureDevice(uint32_t sampleRateHz) {
  if (g_waveOut && g_sampleRate == sampleRateHz) {
    return true;
  }

  ShutdownAudio();

  WAVEFORMATEX fmt{};
  fmt.wFormatTag = WAVE_FORMAT_PCM;
  fmt.nChannels = 1;
  fmt.nSamplesPerSec = sampleRateHz;
  fmt.wBitsPerSample = 16;
  fmt.nBlockAlign = static_cast<WORD>((fmt.nChannels * fmt.wBitsPerSample) / 8);
  fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;
  fmt.cbSize = 0;

  HWAVEOUT hwo = nullptr;
  const UINT rc = waveOutOpen(&hwo, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL);
  if (rc != 0 || !hwo) {
    e00::GetDefaultLogger().Warning(e00::source_location::current(),
                                    "waveOutOpen failed ({})", static_cast<int>(rc));
    return false;
  }

  g_waveOut = hwo;
  g_sampleRate = sampleRateHz;

  for (int i = 0; i < kNumBuffers; ++i) {
    std::memset(&g_buffers[i], 0, sizeof(g_buffers[i]));
    g_buffers[i].hdr.lpData = g_buffers[i].data;
    g_buffers[i].hdr.dwBufferLength = kBufferBytes;
    g_buffers[i].hdr.dwFlags = 0;
    if (waveOutPrepareHeader(g_waveOut, &g_buffers[i].hdr, sizeof(WAVEHDR)) != 0) {
      e00::GetDefaultLogger().Warning(e00::source_location::current(),
                                      "waveOutPrepareHeader failed");
      ShutdownAudio();
      return false;
    }
  }

  g_ready = true;
  g_nextBuffer = 0;
  return true;
}

}// namespace

void InitAudio() {
  g_ready = false;
  g_waveOut = nullptr;
  g_sampleRate = 0;
  g_nextBuffer = 0;
}

void ShutdownAudio() {
  if (g_waveOut) {
    waveOutReset(g_waveOut);
    for (int i = 0; i < kNumBuffers; ++i) {
      if (g_buffers[i].hdr.dwFlags & WHDR_PREPARED) {
        waveOutUnprepareHeader(g_waveOut, &g_buffers[i].hdr, sizeof(WAVEHDR));
      }
      g_buffers[i].inFlight = false;
      g_buffers[i].hdr.dwFlags = 0;
    }
    waveOutClose(g_waveOut);
    g_waveOut = nullptr;
  }
  g_ready = false;
  g_sampleRate = 0;
}

void PumpAudio() {
  if (!g_waveOut) {
    return;
  }
  for (int i = 0; i < kNumBuffers; ++i) {
    if (g_buffers[i].inFlight && (g_buffers[i].hdr.dwFlags & WHDR_DONE)) {
      g_buffers[i].inFlight = false;
      g_buffers[i].hdr.dwFlags &= ~WHDR_DONE;
    }
  }
}

void QueuePcmMono16(std::span<const int16_t> samples, uint32_t sampleRateHz) {
  if (samples.empty() || sampleRateHz == 0) {
    return;
  }
  if (!EnsureDevice(sampleRateHz)) {
    return;
  }

  PumpAudio();

  size_t offset = 0;
  const size_t totalBytes = samples.size() * sizeof(int16_t);
  const auto *srcBytes = reinterpret_cast<const char *>(samples.data());

  while (offset < totalBytes) {
    AudioBuffer *buf = nullptr;
    for (int tries = 0; tries < kNumBuffers; ++tries) {
      AudioBuffer &candidate = g_buffers[g_nextBuffer];
      g_nextBuffer = (g_nextBuffer + 1) % kNumBuffers;
      if (!candidate.inFlight) {
        buf = &candidate;
        break;
      }
    }
    if (!buf) {
      // All buffers busy; drop remaining audio this frame.
      break;
    }

    const size_t chunk = (totalBytes - offset) > kBufferBytes ? kBufferBytes : (totalBytes - offset);
    std::memcpy(buf->data, srcBytes + offset, chunk);
    if (chunk < kBufferBytes) {
      std::memset(buf->data + chunk, 0, kBufferBytes - chunk);
    }
    buf->hdr.dwBufferLength = static_cast<DWORD>(chunk);
    buf->hdr.dwFlags &= ~WHDR_DONE;
    if (waveOutWrite(g_waveOut, &buf->hdr, sizeof(WAVEHDR)) == 0) {
      buf->inFlight = true;
      offset += chunk;
    } else {
      break;
    }
  }
}

}// namespace win31
