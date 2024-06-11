#pragma once

#include <map>
#include <type_traits>
#include <functional>

#include "Detail/TypeId.hpp"

namespace e00 {
/**
 * Components can be used to extend a class without touching the class source code,
 *
 */
class ComponentRegistry {
  struct Component {
    [[nodiscard]] virtual void* Get() const = 0;
    virtual ~Component() = default;
  };

  std::map<type_t, Component*> _list;

public:
  ComponentRegistry() = default;

  virtual ~ComponentRegistry() {
    for (auto &[fst, snd] : _list) {
      delete snd;
      snd = nullptr;
    }
  }

  template<typename T>
  [[nodiscard]] bool HasComponent() const {
    return _list.count(type_id<T>()) > 0;
  }

  template<typename T>
  void RemoveComponent() {
    if (auto& it = _list.find(type_id<T>());
      it != _list.end()) {
      delete it->second;
      it->second = nullptr;
      _list.erase(it);
    }
  }

  template<typename T, typename... Args>
  T *CreateComponent(Args&& ...args) {
    struct TComponent : Component {
      std::unique_ptr<T> t;

      explicit TComponent(std::unique_ptr<T> &&t) : t(std::move(t)) {}
      [[nodiscard]] void* Get() const override { return t.get(); }
      ~TComponent() override = default;
    };

    if (HasComponent<T>()) return nullptr;

    auto tuptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto tcom = new TComponent(std::move(tuptr));
    auto ret = _list.emplace(type_id<T>(), std::move(tcom));

    return static_cast<T*>(ret.first->second->Get());
  }

  template<typename T>
  T* GetComponent() const {
    if (const auto& it = _list.find(type_id<T>());
      it != _list.end()) {
      return static_cast<T*>(it->second->Get());
    }

    return nullptr;
  }
};
}// namespace e00
