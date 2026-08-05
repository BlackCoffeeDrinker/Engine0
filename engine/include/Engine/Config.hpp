#pragma once

#include <cstdlib>

#ifdef _WIN32
#if E00_LIBRARY_EXPORT == 1
#define E00_API __declspec(dllexport)
#else
#define E00_API __declspec(dllimport)
#endif
#else
#define E00_API
#endif

#if __has_builtin(__atomic_add_fetch) && defined(__ATOMIC_RELAXED) && defined(__ATOMIC_ACQ_REL)
#define E00_HAS_BUILTIN_ATOMIC_SUPPORT
#endif

#define E00_ALWAYS_INLINE inline __attribute__((always_inline))

// Global Configs
namespace e00 {
using WorldCoordinateType = uint16_t;
using BitmapSizeType = uint16_t;
using TileIdType = uint16_t;

using atomic_long = long;

namespace detail {
template<typename T>
E00_ALWAYS_INLINE T atomic_refcount_increment(T &t) noexcept {
#ifdef E00_HAS_BUILTIN_ATOMIC_SUPPORT
  return __atomic_add_fetch(&t, 1, __ATOMIC_RELAXED);
#else
  return t += 1;
#endif
}

template<class ValueType>
E00_ALWAYS_INLINE bool atomic_compare_exchange(ValueType *val, ValueType *expected, ValueType after) {
#ifdef E00_HAS_BUILTIN_ATOMIC_SUPPORT
  return __atomic_compare_exchange_n(val, expected, after, true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
  if (*__val == *__expected) {
    *__val = __after;
    return true;
  }
  *__expected = *__val;
  return false;
#endif
}

template<typename T>
E00_ALWAYS_INLINE T atomic_refcount_decrement(T &t) noexcept {
#ifdef E00_HAS_BUILTIN_ATOMIC_SUPPORT
  return __atomic_sub_fetch(&t, 1, __ATOMIC_ACQ_REL);
#else
  return t -= 1;
#endif
}

template<class T>
E00_ALWAYS_INLINE T relaxed_load(T const *t) {
#ifdef E00_HAS_BUILTIN_ATOMIC_SUPPORT
  return __atomic_load_n(t, __ATOMIC_RELAXED);
#else
  return *__value;
#endif
}

constexpr size_t MaxActorsInWorld = 512;
}// namespace detail

using ActorId = uint32_t;

constexpr ActorId ActorHashName(std::string_view str) noexcept {
  ActorId hash = 1257225422U;
  for (const auto c: str) {
    const char up = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    hash ^= static_cast<ActorId>(up);
    hash *= 16777619U;
  }
  return hash;
}

// User-defined literal for ultra-clean code syntax: e00_actor
constexpr ActorId operator""_actor(const char *str, size_t len) noexcept {
  return ActorHashName(std::string_view(str, len));
}

using ResourceId = uint32_t;

constexpr ResourceId HashName(std::string_view str) noexcept {
  ResourceId hash = 2166136261U;
  for (const auto c: str) {
    const char up = (c >= 'a' && c <= 'z') ? (c - 32) : c;
    hash ^= static_cast<ResourceId>(up);
    hash *= 16777619U;
  }
  return hash;
}

// User-defined literal for ultra-clean code syntax: e00_id
constexpr ResourceId operator""_id(const char *str, size_t len) noexcept {
  return HashName(std::string_view(str, len));
}

}// namespace e00
