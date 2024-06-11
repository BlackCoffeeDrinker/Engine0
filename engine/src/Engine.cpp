#include <PrivateInclude.hpp>

#include "InternalActions.hpp"


namespace e00 {
Action Engine::BuiltInAction_Quit() {
  return make_action(impl::EngineAction::Quit);
}

Engine::Engine() : _pImpl(std::make_unique<Engine::Data>()) {
}

Engine::~Engine() = default;

InputEvent Engine::InputBindingForAction(const Action &action) const noexcept {
  for (const auto &[inputEvent, inputAction]: _input_binding)
    if (inputAction == action)
      return inputEvent;

  return {};
}

std::error_code Engine::BindInputEventToAction(const Action &action, InputEvent event) noexcept {
  _input_binding.erase(event);
  _input_binding.try_emplace(event, action);

  return {};
}

bool Engine::IsRunning() const noexcept {
  const auto state = _pImpl->State();
  return state != EngineState::QUIT && state != EngineState::INIT;
}

void Engine::Tick(const std::chrono::milliseconds &delta) noexcept {
  switch (_pImpl->State()) {
    case EngineState::INIT:
      if (const auto ec = _pImpl->Init()) {

        _pImpl->SetState(EngineState::QUIT);
      }
      break;

    case EngineState::FIRST_TICK:
      _pImpl->SetState(EngineState::RUNNING_NORMAL);
      [[fallthrough]];

    case EngineState::RUNNING_NORMAL:
      _pImpl->Tick(delta);
      break;

    case EngineState::PAUSE:
      break;

    case EngineState::QUIT:
      break;
  }
}

void Engine::ExecuteAction(Action action) {
  // TODO
  _pImpl->ExecuteAction(action);
}

void Engine::Draw() noexcept {
  _pImpl->Draw();
}

std::error_code Engine::Init() noexcept {
  return _pImpl->Init();
}

detail::ControlBlock *Engine::MakeResourceContainer(const std::string &name, e00::type_t type, const e00::source_location &from) {
  return GlobalResourceManager()->MakeResourceContainer(name, type, from);
}

}// namespace e00
