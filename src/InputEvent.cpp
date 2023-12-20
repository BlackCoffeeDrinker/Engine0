#include <Engine.hpp>

namespace {

class DefaultInputSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override {
    return "Unknown";
  }
  [[nodiscard]] std::string name(uint16_t value) const override {
    return "Unknown";
  }
};

const DefaultInputSystem defaultInputSystem{};
}// namespace

namespace e00 {
const InputSystem &UnknownInputSystem() noexcept {
  return defaultInputSystem;
}
}// namespace e00
