#pragma once

#include "TypeId.hpp"

#include <cstddef>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

namespace e00 {
// Forward declaration of detail cast dispatcher
namespace detail {
template<typename T>
struct CastHelper;
}// namespace detail

/**
 * Property (BoxedValue wrapper) using Hybrid Small Buffer Optimization (SBO).
 *
 * - Types <= 24 bytes (integers, floats, Vec2D, std::string): Zero allocations (inline).
 * - Pointer types: Zero allocations (stores raw pointer).
 * - Types > 24 bytes: Fallback to dynamic heap allocation.
 */
class Property {
  // 24 bytes fits std::string on 64-bit macOS, 6 pointers on 32-bit DOS, Vec2D, etc.
  static constexpr std::size_t InlineStorageSize = 24;

  enum class StorageType : uint8_t {
    Empty,
    Pointer,// Non-owned raw pointer reference
    Inline, // Owned value stored inline inside _storage (0 dynamic allocations)
    Heap    // Fallback heap allocation for types > InlineStorageSize
  };

  using CopyFn = void (*)(const void *src, void *dst);
  using MoveFn = void (*)(void *src, void *dst);
  using DestroyFn = void (*)(void *ptr) noexcept;

  struct VTable {
    CopyFn copy;
    MoveFn move;
    DestroyFn destroy;
  };

  template<typename T>
  static const VTable *get_vtable() noexcept {
    static constexpr VTable vtable{
        // Copy
        [](const void *src, void *dst) {
          if constexpr (sizeof(T) <= InlineStorageSize) {
            ::new (dst) T(*static_cast<const T *>(src));
          } else {
            *static_cast<T **>(dst) = new T(**static_cast<T *const *>(src));
          }
        },
        // Move
        [](void *src, void *dst) {
          if constexpr (sizeof(T) <= InlineStorageSize) {
            ::new (dst) T(std::move(*static_cast<T *>(src)));
          } else {
            *static_cast<T **>(dst) = *static_cast<T **>(src);
            *static_cast<T **>(src) = nullptr;
          }
        },
        // Destroy
        [](void *ptr) noexcept {
          if constexpr (sizeof(T) <= InlineStorageSize) {
            static_cast<T *>(ptr)->~T();
          } else {
            delete *static_cast<T **>(ptr);
          }
        }};
    return &vtable;
  }

  TypeInfo _info{};
  bool _is_return{false};
  bool _is_const_val{false};
  StorageType _storage_type{StorageType::Empty};
  const VTable *_vtable{nullptr};

  union {
    void *_ptr{nullptr};
    alignas(std::max_align_t) std::byte _storage[InlineStorageSize];
  };

  void destroy() noexcept {
    if (_vtable && _vtable->destroy) {
      _vtable->destroy(static_cast<void *>(_storage));
    }
    _storage_type = StorageType::Empty;
    _vtable = nullptr;
  }

  void copy_from(const Property &other) {
    _info = other._info;
    _is_return = other._is_return;
    _storage_type = other._storage_type;
    _vtable = other._vtable;

    if (_storage_type == StorageType::Pointer) {
      _ptr = other._ptr;
    } else if (_storage_type == StorageType::Inline || _storage_type == StorageType::Heap) {
      if (_vtable && _vtable->copy) {
        _vtable->copy(other._storage, _storage);
      }
    }
  }

  void move_from(Property &&other) noexcept {
    _info = other._info;
    _is_return = other._is_return;
    _storage_type = other._storage_type;
    _vtable = other._vtable;

    if (_storage_type == StorageType::Pointer) {
      _ptr = other._ptr;
      other._ptr = nullptr;
    } else if (_storage_type == StorageType::Inline || _storage_type == StorageType::Heap) {
      if (_vtable && _vtable->move) {
        _vtable->move(other._storage, _storage);
      }
    }
    other._storage_type = StorageType::Empty;
    other._vtable = nullptr;
  }

  template<typename T>
  void init(T &&t) {
    using DecayedT = std::decay_t<T>;

    if constexpr (std::is_pointer_v<DecayedT>) {
      _storage_type = StorageType::Pointer;
      _ptr = const_cast<void *>(reinterpret_cast<const void *>(t));
      _vtable = nullptr;
    } else if constexpr (sizeof(DecayedT) <= InlineStorageSize) {
      static_assert(alignof(DecayedT) <= alignof(std::max_align_t),
                    "Type alignment exceeds inline storage alignment.");
      _storage_type = StorageType::Inline;
      _vtable = get_vtable<DecayedT>();
      ::new (static_cast<void *>(_storage)) DecayedT(std::forward<T>(t));
    } else {
      // Graceful fallback for large structures (> InlineStorageSize bytes)
      _storage_type = StorageType::Heap;
      _vtable = get_vtable<DecayedT>();
      auto *heap_obj = new DecayedT(std::forward<T>(t));
      *reinterpret_cast<DecayedT **>(_storage) = heap_obj;
    }
  }

public:
  /// Default constructor (empty/void)
  Property() : _info(TypeInfo()) {}

  /// Value or Pointer constructor
  template<typename T, typename = std::enable_if_t<!std::is_same_v<Property, std::decay_t<T>>>>
  explicit Property(T &&t, bool t_return_value = false)
      : _info(user_type<T>()),
        _is_return(t_return_value),
        _is_const_val(std::is_const_v<std::remove_reference_t<T>>) {
    init(std::forward<T>(t));
  }

  /// Custom TypeInfo constructor
  template<typename T, typename = std::enable_if_t<!std::is_same_v<Property, std::decay_t<T>>>>
  explicit Property(T &&t, TypeInfo type, bool t_return_value = false)
      : _info(type),
        _is_return(t_return_value),
        _is_const_val(std::is_const_v<std::remove_reference_t<T>>) {
    init(std::forward<T>(t));
  }

  ~Property() {
    destroy();
  }

  Property(const Property &other) {
    copy_from(other);
  }

  Property &operator=(const Property &rhs) {
    if (this != &rhs) {
      destroy();
      copy_from(rhs);
    }
    return *this;
  }

  Property(Property &&other) noexcept {
    move_from(std::move(other));
  }

  Property &operator=(Property &&rhs) noexcept {
    if (this != &rhs) {
      destroy();
      move_from(std::move(rhs));
    }
    return *this;
  }

  // region Type info
  [[nodiscard]] const TypeInfo &get_type_info() const noexcept { return _info; }

  [[nodiscard]] bool is_const() const noexcept { return _is_const_val || _info.is_const(); }
  [[nodiscard]] bool is_reference() const noexcept { return _info.is_reference(); }
  [[nodiscard]] bool is_void() const noexcept { return _info.is_void(); }
  [[nodiscard]] bool is_arithmetic() const noexcept { return _info.is_arithmetic(); }
  [[nodiscard]] bool is_undef() const noexcept { return _info.is_undef(); }
  [[nodiscard]] bool is_pointer() const noexcept { return _info.is_pointer(); }
  [[nodiscard]] bool is_class() const noexcept { return _info.is_class(); }
  [[nodiscard]] bool is_integer() const noexcept { return _info.is_integer(); }

  /// Returns true if the stored type matches target type T (ignoring top-level const/ref).
  template<typename T>
  [[nodiscard]] bool is() const noexcept { return _info.bare_equal_type_info(user_type<T>()); }
  // endregion

  // region Raw type extraction
  [[nodiscard]] const void *get_const_ptr() const noexcept {
    if (_storage_type == StorageType::Pointer) return _ptr;
    if (_storage_type == StorageType::Inline) return static_cast<const void *>(_storage);
    if (_storage_type == StorageType::Heap) return *reinterpret_cast<void *const *>(_storage);
    return nullptr;
  }

  [[nodiscard]] void *get_ptr() const noexcept {
    if (_storage_type == StorageType::Pointer) return _ptr;
    if (_storage_type == StorageType::Inline) return const_cast<void *>(static_cast<const void *>(_storage));
    if (_storage_type == StorageType::Heap) return *reinterpret_cast<void *const *>(_storage);
    return nullptr;
  }
  // endregion

  /**
   * @brief Safe pointer extraction (returns nullptr if type match fails or const check fails).
   * @tparam T Target pointee type.
   * @return T* Pointer to stored value, or nullptr if type mismatch.
   */
  template<typename T>
  [[nodiscard]] const T *as() const noexcept {
    if (is<T>()) {
      return static_cast<const T *>(get_const_ptr());
    }
    return nullptr;
  }

  template<typename T>
  [[nodiscard]] T *as() noexcept {
    if (!is_const() && is<T>()) {
      return static_cast<T *>(get_ptr());
    }
    return nullptr;
  }

  /**
   * @brief Casts property to target type T (Value, Reference, or Pointer).
   * @tparam T Desired output type.
   * @note Aborts the process on type mismatch or const violation (no exceptions are used).
   */
  template<typename T>
  decltype(auto) cast() const { return detail::CastHelper<T>::cast(*this); }

  template<typename T>
  decltype(auto) cast() { return detail::CastHelper<T>::cast(*this); }

  /**
   * @brief Invokes callable `fn` with the casted value if stored type matches T.
   * @tparam T Expected target type.
   * @tparam Callable Function or Lambda taking `T&` or `const T&`.
   * @return true if match succeeded and function was called; false otherwise.
   */
  template<typename T, typename Callable>
  bool try_cast(Callable &&fn) const {
    if (is<T>()) {
      fn(this->cast<const std::remove_reference_t<T> &>());
      return true;
    }
    return false;
  }

  template<typename T, typename Callable>
  bool try_cast(Callable &&fn) {
    if (is<T>()) {
      fn(this->cast<std::remove_reference_t<T> &>());
      return true;
    }
    return false;
  }

  /// Used for explicitly creating a "void" object
  struct VoidType {};
};

inline Property void_var() {
  return Property(Property::VoidType());
}

namespace detail {
// Reference / Value Cast Dispatcher
template<typename T>
struct CastHelper {
  using RawType = std::remove_cvref_t<T>;

  static decltype(auto) cast(const Property &prop) {
    if (!prop.template is<RawType>()) {
      // e00::Property cast error: Type mismatch
      abort();
    }
    if constexpr (std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>) {
      if (prop.is_const()) {
        // e00::Property cast error: Cannot cast const Property to mutable reference
        abort();
      }
    }
    return *static_cast<const RawType *>(prop.get_const_ptr());
  }

  static decltype(auto) cast(Property &prop) {
    if (!prop.template is<RawType>()) {
      // e00::Property cast error: Type mismatch
      abort();
    }
    if constexpr (std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>) {
      if (prop.is_const()) {
        // e00::Property cast error: Cannot cast const Property to mutable reference
        abort();
      }
      return *static_cast<RawType *>(prop.get_ptr());
    } else {
      return *static_cast<const RawType *>(prop.get_const_ptr());
    }
  }
};

// Specialization for Pointer Types (T*)
template<typename T>
struct CastHelper<T *> {
  static T *cast(Property &prop) {
    return prop.template as<T>();
  }

  static const T *cast(const Property &prop) {
    return prop.template as<T>();
  }
};

}// namespace detail
}// namespace e00
