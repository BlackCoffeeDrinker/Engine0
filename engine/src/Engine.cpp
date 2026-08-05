#include "ActorLoader.hpp"
#include "PrivateInclude.hpp"

#include "InternalActions.hpp"
#include "TranslatableText.hpp"
#include "WorldLoader.hpp"

#include <Engine/World.hpp>

#include <memory>
#include <ranges>

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
      _strings(std::make_unique<TranslatableText>()),
      _platform_data(nullptr) {
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
  auto &resource_manager = ResourceManager::GlobalResourceManager();
  if (const auto worldIni = resource_manager.FindStreamForResource(HashName(world_name))) {
    impl::WorldLoader loader(&resource_manager);
    const auto worldData = loader.Load(*worldIni);
    if (worldData.height == 0 || worldData.width == 0) {
      GetDefaultLogger().Error(source_location::current(), "Failed to load world {}, invalid size", world_name);
      return std::make_error_code(std::errc::invalid_argument);
    }
    if (worldData.tilesets.empty()) {
      GetDefaultLogger().Error(source_location::current(), "Failed to load world {}, no tilesets", world_name);
      return std::make_error_code(std::errc::invalid_argument);
    }
    if (worldData.groundSet.empty()) {
      GetDefaultLogger().Error(source_location::current(), "Failed to load world {}, no ground set", world_name);
      return std::make_error_code(std::errc::invalid_argument);
    }

    auto world = std::unique_ptr<World>(new World(
        world_name,
        {static_cast<WorldCoordinateType>(worldData.width), static_cast<WorldCoordinateType>(worldData.height)}));
    for (const auto &tileset: worldData.tilesets) {
      // TODO: Same tileset as before (maybe with different startTileId) ? Reuse and don't reload

      world->AddTileset(tileset.startTileId, resource_manager.LazyResource<Tileset>(HashName(tileset.tilesetResourceName)));
    }

    // Copy the ground & above
    const std::array layers{&worldData.groundSet, &worldData.aboveSet};

    for (size_t layer = 0; layer < layers.size(); layer++) {
      for (size_t i = 0; i < layers[layer]->size(); ++i) {
        const auto &value = layers[layer]->at(i);
        const TilePosition position(i % worldData.width, i / worldData.width);
        if (!world->SetMapTile(layer, position, value)) {
          return std::make_error_code(std::errc::invalid_argument);
        }
      }
    }

    // Do the actors
    impl::ActorLoader actorLoader(&resource_manager);
    for (const auto &[name, def]: worldData.actors) {
      const auto resourceId = HashName(def.source);

      if (!_actors.contains(resourceId)) {
        if (const auto actorIni = resource_manager.FindStreamForResource(resourceId)) {
          if (auto actor = actorLoader.LoadActor(*actorIni)) {
            _actors.emplace(resourceId, std::move(actor));
          } else {
            // Something went wrong, log it
            GetDefaultLogger().Error(source_location::current(), "Failed to load actor {}", name);
            return std::make_error_code(std::errc::invalid_argument);
          }
        } else {
          // Something went wrong, log it
          GetDefaultLogger().Error(source_location::current(), "Failed find source for actor {}: {}", name, def.source);
          return std::make_error_code(std::errc::invalid_argument);
        }
      }

      // Loaded and need to be registered as `name`
      world->Insert(
          name,
          _actors.at(resourceId).get(),
          def.position);
    }

    if (_current_world) {
      // TODO: _script_engine->call<...>("world_unload")
      OnWorldUnload(_current_world);

      // Make a list of used actors in this new world
      std::vector<ResourceId> actorsUsed(worldData.actors.size());
      for (const auto &name: worldData.actors | std::views::keys) {
        actorsUsed.push_back(HashName(name));
      }

      // Make a list of actors to unload
      std::vector<ResourceId> actorsToUnload;
      for (const auto &name: _actors | std::views::keys) {
        if (!std::ranges::contains(actorsUsed, name)) {
          actorsToUnload.push_back(name);
        }
      }

      // Unload actors that aren't used in this new world
      for (const auto &name: actorsToUnload) {
        _actors.erase(name);
      }

      _current_world.reset();
    }

    _current_world = std::move(world);
    OnWorldLoaded(_current_world);
    return {};
  }

  GetDefaultLogger().Error(source_location::current(), "Failed to load world {}", world_name);
  return std::make_error_code(std::errc::invalid_argument);
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
        _current_world->Tick(delta);
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
