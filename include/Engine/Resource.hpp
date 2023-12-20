#pragma once

namespace e00 {
class Engine;

struct Resource {
  Resource() = default;
  Resource(const Resource &other) = default;
  Resource(Resource &&rhs) noexcept : _parent(rhs._parent) { rhs._parent = nullptr; }

  Resource &operator=(const Resource &other) {
    if (this != &other) {
      _parent = other._parent;
    }

    return *this;
  }

  Resource &operator=(Resource &&other) noexcept {
    if (this != &other) {
      _parent = other._parent;
      other._parent = nullptr;
    }
    return *this;
  }

  [[nodiscard]] auto GetParentEngine() const { return _parent; }

  void SetParentEngine(Engine *p) { _parent = p; }

  virtual ~Resource() = default;

private:
  Engine *_parent = nullptr;
};
}// namespace e00
