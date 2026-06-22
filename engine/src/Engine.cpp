#include <PrivateInclude.hpp>

#include "InternalActions.hpp"
#include "TranslatableText.hpp"

namespace e00 {
Action Engine::BuiltInAction_Quit() {
  return make_action(impl::EngineAction::Quit);
}

Action Engine::BuiltInAction_PauseToggle() {
  return make_action(impl::EngineAction::PauseToggle);
}

void Engine::ExecuteActionsAtTime(const GameClock::time_point &tp) {
  // Drain the list until time_point tp
  while (!_actions_to_execute.empty() && _actions_to_execute.top().when <= tp) {
    const auto &topAction = _actions_to_execute.top();

    // Is the root widget interested in this action?
    if (const auto widgetProcessResult = _root_widget->ProcessAction(topAction);
        widgetProcessResult != Widget::ActionProcessResult::HandledAndConsumed) {
      if (!_current_world || !_current_world->ProcessAction(topAction)) {
        ExecuteAction(topAction.action);
      }
    }

    _actions_to_execute.pop();
  }
}

Engine::Engine()
    : _state(EngineState::FIRST_TICK),
      _old_state(EngineState::FIRST_TICK),
      _script_engine(ScriptEngine::Create()),
      _root_widget(std::make_unique<Widget>()),
      _strings(std::make_unique<TranslatableText>()) {
  GetDefaultLogger().Verbose(source_location::current(), "E0 Starting");

  _script_engine->register_function("GetText", [this](int textCode) -> std::string {
    return _strings->GetText(textCode);
  });
}

Engine::~Engine() = default;

std::error_code Engine::AddText(const std::string &locale, int textCode, const std::string &text) {
  return {};
}


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
  return _state != EngineState::QUIT;
}

bool Engine::IsPaused() const noexcept {
  return _state == EngineState::PAUSE;
}

std::error_code Engine::LoadWorld(const std::string &world_name) {
  if (_current_world) {
    // TODO: _script_engine->call<...>("world_unload")
    OnWorldUnload(_current_world);
    _current_world.reset();
  }

  GetDefaultLogger().Verbose(source_location::current(), "Loading world: {}", world_name);

  auto &resource_manager = ResourceManager::GlobalResourceManager();
  auto map = resource_manager.LoadResourceDirectly<Map>(HashName(world_name), DiscardPalette{});
  if (!map) {
    GetDefaultLogger().Error(source_location::current(), "Failed to load map {}", world_name);
    return std::make_error_code(std::errc::invalid_argument);
  }

  _current_world = std::make_unique<World>(world_name);
  std::ignore = _current_world->AddMap(std::move(map));

  GetDefaultLogger().Verbose(source_location::current(), "World loaded: {}", world_name);
  OnWorldLoaded(_current_world);

  return {};
}

void Engine::Tick(const std::chrono::milliseconds &delta) noexcept {
  switch (_state) {
    case EngineState::FIRST_TICK:
      _state = EngineState::RUNNING_NORMAL;
      OnFirstTick();
      [[fallthrough]];

    case EngineState::RUNNING_NORMAL:
      if (_old_state == EngineState::PAUSE) {
        OnResume();
      }
      _current_game_time += delta;

      if (_current_world) {
        //
      }

      ExecuteActionsAtTime(Now());

      break;

    case EngineState::PAUSE:
      if (_old_state == EngineState::RUNNING_NORMAL) {
        OnPause();
      }
      ExecuteActionsAtTime(Now());
      break;

    case EngineState::QUIT:
      break;
  }

  _old_state = _state;
}
void Engine::ProcessInputEvent(const InputEvent event) {
  GetDefaultLogger().Info(
      source_location::current(),
      "Input event {} received",
      event.message());

  if (const auto &event_binding = _input_binding.find(event);
      event_binding != _input_binding.end()) {
    QueueActionForNextTick(event_binding->second);
  }
}

void Engine::QueueActionForNextTick(Action action) {
  _actions_to_execute.emplace(action, Now());
}

Widget *Engine::RootWidget() {
  return _root_widget.get();
}

std::error_code Engine::Init() noexcept {
  _current_game_time = GameClock::time_point();
  _state = _old_state = EngineState::FIRST_TICK;

  return OnInit();
}

void Engine::ExecuteAction(const Action &action) {

  if (action == BuiltInAction_Quit()) {
    GetDefaultLogger().Info(source_location::current(), "Action EXIT: Quitting");
    _state = (EngineState::QUIT);
  } else if (action == BuiltInAction_PauseToggle()) {
    if (_state == EngineState::PAUSE) {
      _state = (EngineState::RUNNING_NORMAL);
    } else {
      _state = (EngineState::PAUSE);
    }
  }
}

}// namespace e00
