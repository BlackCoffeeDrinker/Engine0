#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <system_error>
#include <type_traits>

#include <Engine/Config.hpp>
#include <Engine/Detail/TypeId.hpp>
#include <Engine/Resource.hpp>

namespace e00 {
namespace detail {
class ControlBlock {
  const ResourceId _id;
  const type_t _type;
  atomic_long _strong_ref_count;

protected:
  Resource *_resource = nullptr;
  virtual void OnZeroShared() noexcept = 0;

public:
  constexpr ControlBlock(ResourceId id, type_t type, long strong_ref_count = 0)
      : _id(id),
        _type(type),
        _strong_ref_count(strong_ref_count) {}

  ControlBlock(ControlBlock &&) noexcept = delete;
  virtual ~ControlBlock() = default;

  [[nodiscard]] ResourceId id() const { return _id; }
  [[nodiscard]] type_t type() const { return _type; }
  [[nodiscard]] E00_ALWAYS_INLINE long UseCount() const noexcept { return relaxed_load(&_strong_ref_count); }
  E00_ALWAYS_INLINE void AddStrongRef() noexcept { atomic_refcount_increment(_strong_ref_count); }
  E00_ALWAYS_INLINE bool RemoveStrongRef() noexcept {
    if (atomic_refcount_decrement(_strong_ref_count) == 0) {
      OnZeroShared();
      return true;
    }

    return false;
  }

  [[nodiscard]] Resource *resource() {
    if (!_resource) { (void) OnLoadLazyResource(); }
    return _resource;
  }

  [[nodiscard]] bool IsLoaded() const { return _resource != nullptr; }
  virtual std::error_code OnLoadLazyResource() = 0;

  explicit operator bool() const noexcept { return _id != 0 && _type != type_t{}; }
  [[nodiscard]] bool operator==(std::nullptr_t) const { return _id == 0 && _type == type_t{}; }
  [[nodiscard]] bool operator!=(std::nullptr_t) const { return _id != 0 || _type != type_t{}; }
  [[nodiscard]] bool operator==(const ControlBlock &other) const noexcept { return id() == other.id() && type() == other.type() && this == &other; }
  [[nodiscard]] bool operator!=(const ControlBlock &other) const noexcept { return id() != other.id() || type() != other.type() || this != &other; }
};

class InvalidControlBlock : public ControlBlock {
  constexpr InvalidControlBlock() : ControlBlock(0, 0, 0) {}

protected:
  void OnZeroShared() noexcept override {}

public:
  std::error_code OnLoadLazyResource() override { return std::make_error_code(std::errc::function_not_supported); }

  static constexpr InvalidControlBlock *InvalidBlock() {
    static InvalidControlBlock block{};
    return &block;
  }
};
}// namespace detail

template<typename T>
struct ResourcePtrT {
  static_assert(std::is_base_of_v<Resource, T>, "Class must be of type resource");
  using element_type = T;
  using pointer = std::remove_reference_t<std::remove_cv_t<T>> *;
  using reference = std::remove_reference_t<std::remove_cv_t<T>> &;
  using const_reference = std::add_const_t<reference>;

  template<typename U>
  friend struct ResourcePtrT;

  ResourcePtrT() : cb(detail::InvalidControlBlock::InvalidBlock()) {}
  ResourcePtrT(std::nullptr_t) : cb(detail::InvalidControlBlock::InvalidBlock()) {}
  ResourcePtrT(const ResourcePtrT &copy) : cb(copy.cb) { AddRef(); }
  ResourcePtrT(ResourcePtrT &&move) noexcept : cb(move.cb) { move.cb = detail::InvalidControlBlock::InvalidBlock(); }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  ResourcePtrT(const ResourcePtrT<U> &copy) : cb(copy.cb) { AddRef(); }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  ResourcePtrT(ResourcePtrT<U> &&move) noexcept : cb(move.cb) { move.cb = detail::InvalidControlBlock::InvalidBlock(); }

  ~ResourcePtrT() { RemoveRef(); }

  ResourcePtrT &operator=(const ResourcePtrT &other) {
    if (this != &other) {
      RemoveRef();
      cb = other.cb;
      AddRef();
    }
    return *this;
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  ResourcePtrT &operator=(const ResourcePtrT<U> &other) {
    RemoveRef();
    cb = other.cb;
    AddRef();
    return *this;
  }

  ResourcePtrT &operator=(ResourcePtrT &&other) noexcept {
    if (this != &other) {
      RemoveRef();
      cb = other.cb;
      other.cb = detail::InvalidControlBlock::InvalidBlock();
    }
    return *this;
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  ResourcePtrT &operator=(ResourcePtrT<U> &&other) noexcept {
    if (this != &other) {
      RemoveRef();
      cb = other.cb;
      other.cb = detail::InvalidControlBlock::InvalidBlock();
    }
    return *this;
  }

  ResourcePtrT &operator=(std::nullptr_t) {
    RemoveRef();
    cb = detail::InvalidControlBlock::InvalidBlock();
    return *this;
  }

  [[nodiscard]] bool operator==(const ResourcePtrT &other) const { return *cb == *other.cb; }
  [[nodiscard]] bool operator!=(const ResourcePtrT &other) const { return *cb != *other.cb; }
  [[nodiscard]] bool operator==(std::nullptr_t) const { return cb == detail::InvalidControlBlock::InvalidBlock(); }
  [[nodiscard]] bool operator!=(std::nullptr_t) const { return cb != detail::InvalidControlBlock::InvalidBlock(); }
  [[nodiscard]] explicit operator bool() const { return cb != detail::InvalidControlBlock::InvalidBlock(); }

  [[nodiscard]] ResourceId Id() const { return cb->id(); }
  [[nodiscard]] pointer get() const { return static_cast<T *>(cb->resource()); }
  [[nodiscard]] const_reference operator*() const { return *get(); }
  [[nodiscard]] reference operator*() { return *get(); }
  [[nodiscard]] pointer operator->() const { return get(); }
  [[nodiscard]] const_reference Ref() const { return *get(); }

  std::error_code EnsureLoad() const { return !cb->IsLoaded() ? cb->OnLoadLazyResource() : std::error_code(); }

private:
  friend class ResourceManager;
  detail::ControlBlock *cb;

  explicit ResourcePtrT(detail::ControlBlock *cb_ptr)
      : cb(cb_ptr ? cb_ptr : detail::InvalidControlBlock::InvalidBlock()) { AddRef(); }

  void AddRef() const { cb->AddStrongRef(); }
  void RemoveRef() const { cb->RemoveStrongRef(); }
};

}// namespace e00
