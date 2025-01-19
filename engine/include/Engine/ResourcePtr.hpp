#pragma once

#include <cstddef>

#include "Resource.hpp"

namespace e00 {
class Engine;

namespace detail {
  struct ControlBlock {
    virtual ~ControlBlock() = default;

    virtual void add_ref() noexcept = 0;
    virtual void remove_ref() noexcept = 0;

    virtual Resource *resource() = 0;

    [[nodiscard]] virtual std::string_view name() const = 0;
    [[nodiscard]] virtual type_t type() const = 0;
  };
}// namespace detail

template<typename T>
struct ResourcePtrT {
  static_assert(std::is_base_of_v<Resource, T>, "Class must be of type resource");
  using pointer = std::remove_reference_t<std::remove_cv_t<T>> *;
  using reference = std::remove_reference_t<std::remove_cv_t<T>> &;
  using const_reference = std::add_const_t<reference>;

  ResourcePtrT() = default;
  ResourcePtrT(std::nullptr_t) {}
  ResourcePtrT(const ResourcePtrT &copy) : cb(copy.cb) { AddRef(); }
  ResourcePtrT(ResourcePtrT &&move) noexcept : cb(move.cb) { move.cb = nullptr; }

  ~ResourcePtrT() {
    RemoveRef();
  }

  ResourcePtrT &operator=(const ResourcePtrT &other) {
    if (this != &other) {
      RemoveRef();
      cb = other.cb;
      AddRef();
    }
    return *this;
  }

  ResourcePtrT &operator=(ResourcePtrT &&other) noexcept {
    if (this != &other) {
      RemoveRef();
      cb = other.cb;
      other.cb = nullptr;
    }
    return *this;
  }

  ResourcePtrT &operator=(nullptr_t other) {
    if (other != this) {
      RemoveRef();
    }
    return *this;
  }

  bool operator==(const ResourcePtrT &other) const {
    return cb != nullptr && other.cb == cb;
  }

  bool operator!=(const ResourcePtrT &other) const {
    return cb == nullptr || cb != other.cb;
  }

  explicit operator bool() const { return cb != nullptr; }

  [[nodiscard]] std::string_view Name() const {
    return cb ? cb->name() : "";
  }

  pointer Get() const {
    if (!cb) return nullptr;
    return static_cast<T *>(cb->resource());
  }

  const_reference operator*() const { return Get(); }
  reference operator*() { return Get(); }
  pointer operator->() const { return Get(); }

  std::error_code EnsureLoad() {
    (void)cb->resource();
    return {};
  }

  const_reference Ref() const { return *Get(); }

private:
  friend class ResourceManager;
  detail::ControlBlock *cb;

  explicit ResourcePtrT(detail::ControlBlock *cb) : cb(cb) { AddRef(); }

  void AddRef() {
    if (cb) cb->add_ref();
  }

  void RemoveRef() {
    if (cb) {
      cb->remove_ref();
      cb = nullptr;
    }
  }
};
}// namespace e00
