#pragma once

#include <memory>

#include "Action.hpp"

namespace e00 {
/**
 * A binding is an action to a function
 * Use the method `e00::make_binding` to create one
 */
struct zBinding {
  explicit zBinding(const Action& action) : _action(action) {}

  virtual ~zBinding() = default;

  virtual void run() const noexcept = 0;

  void operator()() const noexcept { run(); }

  [[nodiscard]] const Action &action() const noexcept { return _action; }

private:
  Action _action;
};

/**
 * Creates a binding for Action action to function fn
 *
 * @tparam Fn
 * @param action the action to bind for
 * @param fn the function to bind to action
 * @return the binding
 */
template<typename Fn>
std::unique_ptr<zBinding> make_zbinding(const Action &action, Fn &&fn) {
  using FnD = typename std::decay_t<Fn>;

  struct InternalBinding : FnD, zBinding {
    constexpr InternalBinding(Action action, FnD &&t)
      : FnD(std::forward<FnD>(t)), zBinding(action) {}
    ~InternalBinding() override = default;
    void run() const noexcept override { FnD::operator()(); }
  };

  return std::make_unique<InternalBinding>(action, std::forward<FnD>(fn));
}
}// namespace e00
