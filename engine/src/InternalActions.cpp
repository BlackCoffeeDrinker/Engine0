#include "InternalActions.hpp"

namespace {
struct BuiltInActionsCategory : public e00::ActionCategory {
  [[nodiscard]] std::string_view name() const noexcept override {
    return "Generic";
  }

  [[nodiscard]] std::string_view message(uint32_t binding) const override {
    switch (static_cast<e00::impl::EngineAction>(binding)) {
      case e00::impl::EngineAction::Quit:
        return "Quit";
      case e00::impl::EngineAction::PauseToggle:
        return "Toggle Pause";
    }

    return {};
  }
};

const BuiltInActionsCategory builtInActionsCategory{};
}// namespace

namespace e00::impl {
Action make_action(EngineAction e) {
  return {e, builtInActionsCategory};
}
}// namespace e00::impl
