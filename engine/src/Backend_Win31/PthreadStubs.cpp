// Minimal single-threaded stand-ins for the handful of pthread_* symbols that
// libgcc_eh's SJLJ unwinder (and libstdc++'s internal thread-safety bookkeeping)
// reference, even though our own code never uses real threads and is built
// with -fno-exceptions/-fno-threadsafe-statics. Linking the real "winpthread"
// library instead pulls in its thread.c, which references the PE "_tls_used"
// TLS directory descriptor that only the (deliberately excluded, via
// -nostartfiles) CRT startup object normally provides. Since Win32s itself
// has no real preemptive threading model that the engine relies on (see
// platform::CreateThread, which already just logs and returns
// platform::InvalidThreadId), a single "current thread" model here is
// sufficient and avoids the whole _tls_used problem.
#include "Win32Types.hpp"

#include <cstddef>

namespace {
// A small fixed-size table of "thread-specific" slots. Since there is only
// ever one logical thread, this is just a flat array indexed by key.
constexpr int kMaxKeys = 32;
void *g_tss_values[kMaxKeys] = {};
int g_next_key = 0;
}// namespace

extern "C" {

using pthread_key_t = int;
using pthread_once_t = int;
using pthread_mutex_t = int;

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) {
  (void) destructor;// never called: process teardown is via ExitProcess
  if (key == nullptr || g_next_key >= kMaxKeys) {
    return 1;
  }
  *key = g_next_key++;
  return 0;
}

int pthread_key_delete(pthread_key_t key) {
  if (key < 0 || key >= kMaxKeys) {
    return 1;
  }
  g_tss_values[key] = nullptr;
  return 0;
}

void *pthread_getspecific(pthread_key_t key) {
  if (key < 0 || key >= kMaxKeys) {
    return nullptr;
  }
  return g_tss_values[key];
}

int pthread_setspecific(pthread_key_t key, const void *value) {
  if (key < 0 || key >= kMaxKeys) {
    return 1;
  }
  g_tss_values[key] = const_cast<void *>(value);
  return 0;
}

int pthread_once(pthread_once_t *once_control, void (*init_routine)()) {
  if (once_control != nullptr && *once_control == 0) {
    *once_control = 1;
    if (init_routine != nullptr) {
      init_routine();
    }
  }
  return 0;
}

// Single-threaded: mutexes are always uncontended, so these are no-ops.
int pthread_mutex_init(pthread_mutex_t *, const void *) { return 0; }
int pthread_mutex_destroy(pthread_mutex_t *) { return 0; }
int pthread_mutex_lock(pthread_mutex_t *) { return 0; }
int pthread_mutex_unlock(pthread_mutex_t *) { return 0; }
int pthread_mutex_trylock(pthread_mutex_t *) { return 0; }

}// extern "C"
