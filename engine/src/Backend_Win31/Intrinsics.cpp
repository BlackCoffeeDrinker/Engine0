#include "Win32Types.hpp"

#include <cstddef>
#include <cstdint>

// Minimal C runtime pieces still emitted by gcc/libstdc++ under -ffreestanding/-nostdlib.

extern "C" {

void *memcpy(void *dest, const void *src, size_t n) {
  auto *d = static_cast<unsigned char *>(dest);
  const auto *s = static_cast<const unsigned char *>(src);
  for (size_t i = 0; i < n; ++i) {
    d[i] = s[i];
  }
  return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
  auto *d = static_cast<unsigned char *>(dest);
  const auto *s = static_cast<const unsigned char *>(src);
  if (d == s || n == 0) {
    return dest;
  }
  if (d < s) {
    for (size_t i = 0; i < n; ++i) {
      d[i] = s[i];
    }
  } else {
    for (size_t i = n; i > 0; --i) {
      d[i - 1] = s[i - 1];
    }
  }
  return dest;
}

void *memset(void *dest, int c, size_t n) {
  auto *d = static_cast<unsigned char *>(dest);
  const auto v = static_cast<unsigned char>(c);
  for (size_t i = 0; i < n; ++i) {
    d[i] = v;
  }
  return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const auto *a = static_cast<const unsigned char *>(s1);
  const auto *b = static_cast<const unsigned char *>(s2);
  for (size_t i = 0; i < n; ++i) {
    if (a[i] != b[i]) {
      return static_cast<int>(a[i]) - static_cast<int>(b[i]);
    }
  }
  return 0;
}

size_t strlen(const char *s) {
  size_t n = 0;
  while (s[n] != '\0') {
    ++n;
  }
  return n;
}

char *strcpy(char *dest, const char *src) {
  char *d = dest;
  while ((*d++ = *src++) != '\0') {
  }
  return dest;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }
  return static_cast<unsigned char>(*s1) - static_cast<unsigned char>(*s2);
}

char *strcat(char *dest, const char *src) {
  char *d = dest + strlen(dest);
  while ((*d++ = *src++) != '\0') {
  }
  return dest;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    const unsigned char c1 = static_cast<unsigned char>(s1[i]);
    const unsigned char c2 = static_cast<unsigned char>(s2[i]);
    if (c1 != c2 || c1 == 0) {
      return static_cast<int>(c1) - static_cast<int>(c2);
    }
  }
  return 0;
}

void abort() {
  ExitProcess(3);
}

// Mingw pseudo-reloc runtime hook; not needed for our fixed image layout.
void _pei386_runtime_relocator(void) {
}

// Stack probe used by mingw i386 codegen for large frames.
void __chkstk_ms() {
}

void _chkstk() {
}

void __chkstk() {
}

// Pure virtual call stub (no RTTI/exceptions path).
void __cxa_pure_virtual() {
  OutputDebugStringA("pure virtual call\n");
  ExitProcess(4);
}

// Minimal atexit / static guard support for libstdc++ without full CRT.
using cxa_atexit_fn = void (*)(void *);

struct AtexitEntry {
  cxa_atexit_fn fn;
  void *arg;
};

static constexpr int kMaxAtexit = 64;
static AtexitEntry g_atexit_table[kMaxAtexit];
static int g_atexit_count = 0;
void *__dso_handle = nullptr;

int __cxa_atexit(cxa_atexit_fn fn, void *arg, void * /*dso*/) {
  if (g_atexit_count >= kMaxAtexit) {
    return -1;
  }
  g_atexit_table[g_atexit_count++] = AtexitEntry{fn, arg};
  return 0;
}

int atexit(void (*fn)(void)) {
  return __cxa_atexit(reinterpret_cast<void (*)(void *)>(fn), nullptr, nullptr);
}

void __cxa_finalize(void * /*dso*/) {
  for (int i = g_atexit_count - 1; i >= 0; --i) {
    if (g_atexit_table[i].fn) {
      g_atexit_table[i].fn(g_atexit_table[i].arg);
      g_atexit_table[i].fn = nullptr;
    }
  }
  g_atexit_count = 0;
}

// Local static init guards (compiled with -fno-threadsafe-statics, but some TU may still reference).
int __cxa_guard_acquire(long long *guard) {
  if ((*guard) & 1) {
    return 0;
  }
  return 1;
}

void __cxa_guard_release(long long *guard) {
  *guard = 1;
}

void __cxa_guard_abort(long long * /*guard*/) {
}

} // extern "C"
