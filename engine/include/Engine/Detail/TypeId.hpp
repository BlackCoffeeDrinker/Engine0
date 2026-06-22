#pragma once
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <memory>
#include <functional>

#if defined(__clang__)
#if __has_feature(cxx_rtti)
#define RTTI_ENABLED
#include <typeinfo>
#endif
#elif defined(__GNUG__)
#if defined(__GXX_RTTI)
#define RTTI_ENABLED
#endif
#elif defined(_MSC_VER)
#if defined(_CPPRTTI)
#define RTTI_ENABLED
#endif
#endif

namespace e00 {

using type_t = std::ptrdiff_t;

// A simple constexpr FNV-1a 64-bit hash function
constexpr type_t fnv1a_hash(std::string_view str) {
  unsigned long long hash = 14695981039346656037ULL;
  for (const auto c: str) {
    hash ^= static_cast<unsigned long long>(c);
    hash *= 1099511628211ULL;
  }
  return static_cast<type_t>(hash);
}

constexpr type_t any() {
  return 0;
}

template<typename T>
struct Bare_Type {
  using type = std::remove_cv_t<std::remove_pointer_t<std::remove_reference_t<T>>>;
};

template<typename T>
constexpr std::string_view type_name() {
#ifdef RTTI_ENABLED
  return typeid(T).name();
#else
#ifdef _MSC_VER
  return __FUNCSIG__;
#else
  return __PRETTY_FUNCTION__;
#endif
#endif
}

/*
 The platforms we build for probably don't support RTTI
 (the final executable would be too big for memory)

 So use this instead
 */
template<typename T>
constexpr type_t type_id() {
  return fnv1a_hash(type_name<T>());
}

#undef RTTI_ENABLED

constexpr type_t int_id    = type_id<int>();
constexpr type_t const_int = type_id<const int>();
constexpr type_t ptr_int   = type_id<int*>();

/**
 * Non-polymorphic type information to be held at runtime
 */
class TypeInfo {
  constexpr static unsigned int is_const_flag = 0;
  constexpr static unsigned int is_reference_flag = 1;
  constexpr static unsigned int is_pointer_flag = 2;
  constexpr static unsigned int is_void_flag = 3;
  constexpr static unsigned int is_arithmetic_flag = 4;
  constexpr static unsigned int is_class_flag = 5;
  constexpr static unsigned int is_integer_flag = 7;
  constexpr static unsigned int is_undef_flag = 8;

  unsigned int _flags = (1u << is_undef_flag);
  type_t _type = {};
  type_t _bare = {};
  std::string_view _name = {};

  template<bool Test, unsigned int FlagShift>
  static constexpr unsigned int Flag() {
    return Test ? 1 << FlagShift : 0;
  }

  template<typename T, bool b = std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>, bool>>
  static constexpr bool IsBool() {
    return b;
  }

public:
  // Don't touch this!
  // clang-format off
  // @formatter:off
  template<typename T>
  static constexpr unsigned int MakeFlags() {
    return Flag<std::is_const_v<std::remove_pointer_t<std::remove_reference_t<T>>>, is_const_flag>() +
            Flag<std::is_reference_v<T>, is_reference_flag>() +
            Flag<std::is_pointer_v<T>, is_pointer_flag>() +
            Flag<std::is_void_v<T>, is_void_flag>() +
            Flag<(std::is_arithmetic_v<T> || std::is_arithmetic_v<std::remove_reference_t<T>>) && !IsBool<T>(), is_arithmetic_flag>() +
            Flag<std::is_integral_v<T>, is_integer_flag>() +
            Flag<std::is_class_v<T>, is_class_flag>();
  }
  // @formatter:on
  // clang-format on
  // Carry on formatting

  constexpr TypeInfo() = default;

  constexpr TypeInfo(const TypeInfo &rhs) noexcept = default;

  constexpr TypeInfo(const unsigned int flags, const type_t type, const type_t bare, const std::string_view name) noexcept
      : _flags(flags), _type(type), _bare(bare), _name(name) {
  }

  [[nodiscard]] constexpr bool is_const() const noexcept { return (_flags & (1u << is_const_flag)) != 0; }
  [[nodiscard]] constexpr bool is_reference() const noexcept { return (_flags & (1u << is_reference_flag)) != 0; }
  [[nodiscard]] constexpr bool is_void() const noexcept { return (_flags & (1u << is_void_flag)) != 0; }
  [[nodiscard]] constexpr bool is_arithmetic() const noexcept { return (_flags & (1u << is_arithmetic_flag)) != 0; }
  [[nodiscard]] constexpr bool is_undef() const noexcept { return (_flags & (1u << is_undef_flag)) != 0; }
  [[nodiscard]] constexpr bool is_pointer() const noexcept { return (_flags & (1u << is_pointer_flag)) != 0; }
  [[nodiscard]] constexpr bool is_class() const noexcept { return (_flags & (1u << is_class_flag)) != 0; }
  [[nodiscard]] constexpr bool is_integer() const noexcept { return (_flags & (1u << is_integer_flag)) != 0; }
  [[nodiscard]] constexpr type_t id() const noexcept { return _type; }
  [[nodiscard]] constexpr type_t bare_id() const noexcept { return _bare; }
  [[nodiscard]] constexpr std::string_view name() const noexcept { return _name; }

  constexpr bool operator==(const TypeInfo &rhs) const noexcept { return _type == rhs._type; }
  constexpr bool operator!=(const TypeInfo &rhs) const noexcept { return _type != rhs._type; }
  constexpr bool operator<(const TypeInfo &rhs) const noexcept { return _type < rhs._type; }

  [[nodiscard]] constexpr bool bare_equal_type_info(const TypeInfo &ti) const noexcept {
    return _bare == ti._bare;
  }
};

namespace detail {
template<typename T>
struct GetTypeInfo {
  constexpr static TypeInfo get() noexcept {
    return TypeInfo(
        TypeInfo::MakeFlags<T>(),
        type_id<T>(),
        type_id<typename Bare_Type<T>::type>(),
        type_name<T>());
  }
};

template<typename T>
struct GetTypeInfo<std::unique_ptr<T>> {
  constexpr static TypeInfo get() noexcept {
    return TypeInfo(
        TypeInfo::MakeFlags<T>(),
        type_id<std::unique_ptr<T>>(),
        type_id<typename Bare_Type<T>::type>(),
        type_name<T>());
  }
};

template<typename T>
struct GetTypeInfo<const std::unique_ptr<T> &> : GetTypeInfo<std::unique_ptr<T>> {};

template<typename T>
struct GetTypeInfo<const std::reference_wrapper<T> &> {
  constexpr static TypeInfo get() noexcept {
    return TypeInfo(TypeInfo::MakeFlags<T>(),
                    type_id<const std::reference_wrapper<T> &>(),
                    type_id<typename Bare_Type<T>::type>(),
                    type_name<T>());
  }
};
}// namespace detail

template<typename T>
constexpr TypeInfo user_type() {
  return detail::GetTypeInfo<T>::get();
}

template<typename T>
TypeInfo user_type(const T &) {
  return detail::GetTypeInfo<T>::get();
}

}// namespace e00
