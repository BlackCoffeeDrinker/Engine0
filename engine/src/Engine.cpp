#include "PrivateInclude.hpp"

#include "Actors/ActorChest.hpp"
#include "Actors/ActorDecoration.hpp"
#include "Actors/ActorDoor.hpp"
#include "Actors/ActorMobile.hpp"
#include "Actors/ActorWarpDoor.hpp"

#include "InternalActions.hpp"
#include "TranslatableText.hpp"
#include "WorldLoader.hpp"

#include <Engine/Platform/Stream.hpp>
#include <Engine/Platform/StreamFactory.hpp>

#include <memory>
#include <ranges>

namespace {
using namespace e00;

void ConfigureWarpDoor(impl::ActorWarpDoor &actor, const std::map<std::string, std::string> &properties) {
  if (const auto worldIt = properties.find("target_world"); worldIt != properties.end()) {
    actor.SetTargetWorld(worldIt->second);
  }

  if (const auto entryIt = properties.find("target_entry"); entryIt != properties.end()) {
    actor.SetTargetEntry(entryIt->second);
  }
}

void ConfigureDecoration(impl::ActorDecoration &actor, const std::map<std::string, std::string> &properties) {
  if (const auto collidableIt = properties.find("collidable"); collidableIt != properties.end()) {
    actor.SetCollidable(ParseBool(collidableIt->second, false));
  }
}

void ConfigurePatrolPoints(impl::ActorMobile &actor, const std::map<std::string, std::string> &properties) {
  for (uint32_t i = 1; i <= impl::kMaxPatrolPoints; i++) {
    const auto key = "patrol_" + std::to_string(i);
    const auto it = properties.find(key);
    if (it == properties.end()) {
      break;
    }

    WorldPosition point{};
    if (ParseWorldPoint(it->second, point)) {
      actor.AddPatrolPoint(point);
    }
  }

  if (const auto speedIt = properties.find("speed"); speedIt != properties.end()) {
    int speed = 0;
    if (ParseNumber(speedIt->second, speed)) {
      actor.SetSpeed(static_cast<WorldCoordinateType>(speed));
    }
  }
}

/**
 * A minimal concrete `Actor` with no sprites of its own, used for plain decoration/interaction
 * points (doors, triggers, ...) that don't (yet) have dedicated behavior or a rendered state.
 */
class ActorPlain : public Actor {
public:
  [[nodiscard]] type_t Type() const override { return type_id<ActorPlain>(); }

protected:
  [[nodiscard]] const State &GetCurrentState() const noexcept override {
    static const State kEmptyState{0, {}};
    return kEmptyState;
  }
};
}// namespace

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

std::unique_ptr<Actor> Engine::MakeActorForType(const std::string_view &type, const std::map<std::string, std::string> &properties) {
  if (type == "chest") { return std::make_unique<impl::ActorChest>(); }
  if (type == "door") { return std::make_unique<impl::ActorDoor>(); }

  if (type == "npc") {
    auto actor = std::make_unique<impl::ActorMobile>();
    ConfigurePatrolPoints(*actor, properties);
    return actor;
  }

  if (type == "warp_door") {
    auto actor = std::make_unique<impl::ActorWarpDoor>();
    ConfigureWarpDoor(*actor, properties);
    return actor;
  }

  if (type == "decoration") {
    auto actor = std::make_unique<impl::ActorDecoration>();
    ConfigureDecoration(*actor, properties);
    return actor;
  }

  // Plain interaction points that don't (yet) have dedicated behavior of their own; kept as
  // `ActorPlain` instances so existing world files using these types keep loading.
  if (type == "trigger") { return std::make_unique<ActorPlain>(); }

  return nullptr;
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

std::error_code Engine::LoadWorld(const std::string &world_name, const std::string &entry_point) {
  GetDefaultLogger().Info(source_location::current(), "Loading world {}", world_name);

  auto &resource_manager = ResourceManager::GlobalResourceManager();
  impl::WorldLoader loader(&resource_manager);

  const auto worldIni = resource_manager.FindStreamForResource(HashName(world_name));
  if (!worldIni) {
    GetDefaultLogger().Error(source_location::current(), "Failed to load world {}", world_name);
    return std::make_error_code(std::errc::invalid_argument);
  }

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
    GetDefaultLogger().Info(source_location::current(), "Loading tileset {}", tileset.tilesetResourceName);
    world->AddTileset(tileset.startTileId, resource_manager.LazyResource<Tileset>(HashName(tileset.tilesetResourceName)));
  }

  // Copy the ground & above
  {
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
  }

  // Make a list of used actors in this new world
  std::vector<ActorId> actorsUsed;
  actorsUsed.reserve(worldData.actors.size());

  // Do the actors
  for (const auto &[name, def]: worldData.actors) {
    GetDefaultLogger().Info(source_location::current(), "Loading actor {} ({})", name, def.source);
    const auto actorId = ActorHashName(name);
    actorsUsed.push_back(actorId);

    if (const auto it = _actors.find(actorId);
        it == _actors.end()) {
      if (auto actor = MakeActorForType(def.source, def.default_properties)) {
        if (actor->HasSavableState()) {
          // TODO: Maybe load last state
        }

        if (auto [it, inserted] = _actors.try_emplace(actorId, std::move(actor)); inserted) {
          world->Insert(
              actorId,
              it->second.get(),
              def.position);
        } else {
          GetDefaultLogger().Error(source_location::current(), "Out of free slots for {}", name);
          return std::make_error_code(std::errc::not_enough_memory);
        }

      } else {
        GetDefaultLogger().Error(source_location::current(), "Failed to load actor {}", name);
        return std::make_error_code(std::errc::invalid_argument);
      }
    } else {
      // Loaded and need to be registered as `name`
      world->Insert(
          actorId,
          it->second.get(),
          def.position);
    }
  }

  for (const auto &[name, position]: worldData.entries) {
    // Enter/Exit points
  }

  GetDefaultLogger().Info(source_location::current(), "Loading world complete, replacing current world");
  
  if (_current_world) {
    // TODO: _script_engine->call<...>("world_unload")
    OnWorldUnload(_current_world);

    // TODO: Move the player to the new world

    // Unload actors that aren't used in this new world
    std::vector<ActorId> actorsToRemove;

    for (const auto &entry: _actors) {
      if (!std::ranges::contains(actorsUsed, entry.first)) {
        if (entry.second->HasSavableState()) {
          // TODO: Save actor state
        }

        actorsToRemove.push_back(entry.first);
      }
    }

    for (auto &actorId: actorsToRemove) {
      _actors.erase(actorId);
    }

    _current_world.reset();
  }

  _current_world = std::move(world);
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
