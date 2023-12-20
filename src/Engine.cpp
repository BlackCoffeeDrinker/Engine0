#include <Engine.hpp>

#include "EngineError.hpp"
#include "InternalActions.hpp"

#include "LoadedWorld.hpp"

#include "Loaders/WorldLoader.hpp"
#include "Loaders/BitmapLoader.hpp"

#include "IniParser.hpp"

namespace {
std::unique_ptr<e00::ResourceLoader> CreateLoaderForType(const e00::type_t resourceType) {
  if (resourceType == e00::type_id<e00::Map>()) {
    return std::make_unique<e00::impl::WorldLoader>();
  } else if (resourceType == e00::type_id<e00::Bitmap>()) {
    return std::make_unique<e00::impl::BitmapLoader>();
  }

  return nullptr;
}
}// namespace

namespace e00 {
Action Engine::BuiltInAction_Quit() {
  return make_action(impl::EngineAction::Quit);
}

Engine::Engine() : _main_logger(CreateSink("Engine")),
                   _state(EngineState::INIT),
                   _script_engine(ScriptEngine::Create()) {
  _main_logger.Verbose(source_location::current(), "E0 Starting");
}

Engine::~Engine() = default;

detail::ControlBlock *Engine::MakeResourceContainer(const std::string &name, type_t type, const source_location &from) {
  for (const auto &a : _loaded_resources_cb) {
    if (a->type() == type && a->name() == name) {
      _main_logger.Info(source_location::current(), "Resource {} of type {} already known", name, type);
      return a.get();
    }
  }

  class R : public detail::ControlBlock {
    Engine *_owner;
    type_t _type;
    std::string _name;
    uint32_t _refs;
    std::unique_ptr<Resource> _resource;

  public:
    R(Engine *e, type_t t, std::string n, source_location /*s*/)
      : _owner(e),
        _type(t),
        _name(std::move(n)),
        _refs(0),
        _resource(nullptr) /*, _init(s) */ {
    }

    ~R() override = default;
    [[nodiscard]] std::string name() const override { return _name; }
    [[nodiscard]] type_t type() const override { return _type; }

    void add_ref() noexcept override { _refs++; }

    void remove_ref() noexcept override {
      if (_refs > 0) {
        _refs--;
      }

      if (_refs == 0) {
        _owner->_main_logger.Info(source_location::current(), "Unloading {}", _name);

        // Find ourselves
        auto i = std::find_if(
          _owner->_loaded_resources_cb.begin(),
          _owner->_loaded_resources_cb.end(),
          [this](const auto &uptr) {
            return uptr.get() == this;
          });

        if (i != _owner->_loaded_resources_cb.end()) {
          _owner->_loaded_resources_cb.erase(i);
        } else {
          _owner->_main_logger.Error(source_location::current(), "Failed to find cache for resource {}", _name);
        }
      }
    }

    Resource *resource() override {
      if (!_resource) {
        auto loader = CreateLoaderForType(_type);

        // Complain if we don't have a loader
        if (!loader) {
          _owner->_main_logger.Error(source_location::current(), "Unable to find resource loader");
          return nullptr;
        }

        // Set up the loader
        loader->SetResourceLoader(_owner);

        // Load it (well, try)
        if (auto stream = _owner->OpenStream(_name)) {
          auto result = loader->ReadLoad(*stream);
          if (result.error) {
            _owner->_main_logger.Error(source_location::current(),
              "Resource loader failed to load resource: {}",
              result.error.message());
            return nullptr;
          }

          _resource = std::move(result.resource);
          _resource->SetParentEngine(_owner);
          _owner->_main_logger.Info(source_location::current(), "Resource {} loaded", _name);
        }

        // Something went wrong :(
        _owner->_main_logger.Error(source_location::current(), "Unable to load resource {}: data was not found", _name);
        return nullptr;
      }

      return _resource.get();
    }
  };

  // We didn't find it so make a new container
  auto const &ret = _loaded_resources_cb.emplace_back(std::make_unique<R>(this, type, name, from));

  _main_logger.Info(source_location::current(),
    "New resource {} of type {} asked at {}:{}",
    name,
    type,
    from.file_name(),
    from.line());

  return ret.get();
}

InputEvent Engine::InputBindingForAction(const Action &action) const noexcept {
  for (const auto &[inputEvent, inputAction] : _input_binding)
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
  return _state != EngineState::QUIT
         && _state != EngineState::INIT;
}

void Engine::Tick(const std::chrono::milliseconds &delta) noexcept {
  // Check what we're at
  switch (_state) {
    case EngineState::INIT:
    case EngineState::QUIT:
      return;

    case EngineState::PAUSE:
      // TODO: Pause logic
      break;

    case EngineState::FIRST_TICK:
      _main_logger.Info(source_location::current(), "First tick, game: {}", Name());
      OnFirstTick();
      _state = EngineState::RUNNING_NORMAL;
      [[fallthrough]];

    case EngineState::RUNNING_NORMAL:
      // Update engine time
      _current_time += delta;

      // Process actions
      _actions_to_process.ForEach([this](const auto &action) {
        // Send action to the current state
      });

      if (_current_world) {
        _current_world->Tick(delta);
      }
      break;
  }
}

void Engine::Draw() noexcept {
  // Draw world

  // Draw actors

  // Draw UI elements
}

std::error_code Engine::Init() noexcept {
  // Make sure we're in the right state
  if (_state != EngineState::INIT) {
    _main_logger.Error(source_location::current(), "Trying to re-initialize engine");
    return {};
  }

  // Read the game INI
  _main_logger.Verbose(source_location::current(), "Loading main INI");
  if (auto config = OpenStream("game.ini")) {
    impl::IniParser::Parse(config, [this](const impl::IniParser::Item &item) -> std::error_code {
      _main_logger.Info(source_location::current(), "> {} - {} = {}", item.category, item.key, item.value);
      return {};
    });

  } else {
    _state = EngineState::QUIT;
    _main_logger.Error(source_location::current(), "Failed to load resource game.ini");
    return impl::make_error_code(impl::EngineErrorCode::bad_configuration_file);
  }

  // Make sure the subclass has a chance to init
  if (const auto ec = RealInit()) {
    _state = EngineState::QUIT;
    return ec;
  }

  // Set next state
  _state = EngineState::FIRST_TICK;
  return {};
}
}// namespace e00
