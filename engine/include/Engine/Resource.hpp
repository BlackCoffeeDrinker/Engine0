#pragma once

namespace e00 {
class Engine;

struct Resource {

  virtual ~Resource() = default;

  virtual void Tick(const std::chrono::milliseconds& delta) {
    // Implemented when needed by resources
  }

private:
};
}// namespace e00
