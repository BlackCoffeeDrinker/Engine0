#include <PrivateInclude.hpp>

#include "InternalActions.hpp"

namespace e00 {
Action Engine::BuiltInAction_Quit() {
  return make_action(impl::EngineAction::Quit);
}

Action Engine::BuiltInAction_PauseToggle() {
  return make_action(impl::EngineAction::PauseToggle);
}

Engine::Engine() : _main_logger(CreateSink("Engine")),
                   _pImpl(std::make_unique<Data>(this)) {
}

Engine::~Engine() = default;

bool Engine::LanguageCode(const std::string &languageCode) {
  return true;
}

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
  return _pImpl->State() != EngineState::QUIT;
}

bool Engine::IsPaused() const noexcept {
  return _pImpl->State() == EngineState::PAUSE;
}

void Engine::Tick(const std::chrono::milliseconds &delta) noexcept {
  switch (_pImpl->State()) {
    case EngineState::FIRST_TICK:
      _pImpl->SetState(EngineState::RUNNING_NORMAL);
      OnFirstTick();
      [[fallthrough]];

    case EngineState::RUNNING_NORMAL:
      if (_pImpl->OldState() == EngineState::PAUSE) {
        OnResume();
      }
      _current_game_time += delta;
      _pImpl->TickWorld(delta);
      _pImpl->ExecuteActionsAtTime(Now());
      
      break;

    case EngineState::PAUSE:
      if (_pImpl->OldState() == EngineState::RUNNING_NORMAL) {
        OnPause();
      }
      _pImpl->ExecuteActionsAtTime(Now());
      break;

    case EngineState::QUIT:
      break;
  }

  _pImpl->ClearOldState();
}

void Engine::QueueActionForNextTick(Action action) {
  _pImpl->QueueAction(action, Now());
}

Widget &Engine::RootWidget() {
  return _pImpl->RootWidget();
}

std::error_code Engine::Init() noexcept {
  _current_game_time = GameClock::time_point();
  _pImpl->SetState(EngineState::FIRST_TICK);
  _pImpl->ClearOldState();

  return OnInit();
}

void Engine::ExecuteAction(const Action &action) {

}

}// namespace e00
