#pragma once

/*
 * Includes are here to keep the static analysis tool happy
 */
#include <chrono>
#include <map>
#include <memory>
#include <system_error>

#include "Detail/CircularBuffer.hpp"

#include "Action.hpp"
#include "Binding.hpp"
#include "InputEvent.hpp"

#include "Scripting/ScriptEngine.hpp"

namespace e00 {
enum class EngineState {
  INIT,
  FIRST_TICK,
  RUNNING_NORMAL,
  PAUSE,

  QUIT
};


/**
 * Root engine class, you should subclass this for your game logic
 */
class Engine {
  class LoadedWorld;
  friend LoadedWorld;

  Logger _main_logger;

  EngineState _state;
  GameClock::time_point _current_time;//< Current game time
  detail::CircularBuffer<ActionInstance, 254> _actions_to_process;//< Actions to be executed next tick
  std::map<InputEvent, Action> _input_binding;//< Input event to actions
  std::unique_ptr<LoadedWorld> _current_world;//< Currently main world
  std::unique_ptr<ScriptEngine> _script_engine;//< Persistent script engine
  std::list<std::unique_ptr<detail::ControlBlock>> _loaded_resources_cb;// << TODO: Find better container

  detail::ControlBlock *MakeResourceContainer(const std::string &name, type_t type, const source_location &from);

protected:
  explicit Engine();

  virtual std::error_code RealInit() = 0;

  /**
   * Called only once
   */
  virtual void OnFirstTick() {
    /* Nothing needs to be done here, this is left for subclasses */
  }

  /**
   * Called an actor was created, loaded into memory and placed into an active world
   * @param actor the actor that was loaded
   */
  virtual void OnActorLoaded(Actor &actor) {}

  /**
   * Finds a stream of name
   *
   * @param name
   * @return
   */
  std::unique_ptr<Stream> OpenStream(const std::string& name);

public:
  NOT_COPYABLE(Engine);

  /**
   *
   * @return the built in "Quit" action
   */
  static Action BuiltInAction_Quit();

  /**
   *
   */
  virtual ~Engine();

  /**
   * Must be called by the platform code after
   *
   * @return any error code
   */
  std::error_code Init() noexcept;

  /**
   * Returns the code name of the game
   *
   * @return the code name of the game
   */
  [[nodiscard]] virtual std::string_view Name() const noexcept = 0;

  /**
   * Retrieves the associated input bound to this action
   *
   * @param action the action to query
   * @return the input event associated with this action
   */
  [[nodiscard]] InputEvent InputBindingForAction(const Action &action) const noexcept;

  /**
   * Binds an InputEvent to an Action
   *
   * @param action the target action
   * @param event the input event associated
   * @return error, if any
   */
  std::error_code BindInputEventToAction(const Action &action, InputEvent event) noexcept;

  /**
   * Queries if this engine instance is valid
   *
   * @return true if this engine instance is valid
   */
  [[nodiscard]] bool IsRunning() const noexcept;

  /**
   * Processes a delta tick
   *
   * @param delta time since last tick
   */
  void Tick(const std::chrono::milliseconds &delta) noexcept;

  /**
   * Draw the current state to the screen
   */
  void Draw() noexcept;

  /**
   * Informs this instance's engine that an Input has been received
   *
   * @param event the event
   */
  void ProcessInputEvent(const InputEvent event) {
    if (const auto &event_binding = _input_binding.find(event);
      event_binding != _input_binding.end()) {
      ExecuteAction(event_binding->second);
    }
  }

  /**
   * Unconditionally execute this action on the next tick
   *
   * @param action the action
   */
  void ExecuteAction(const Action action) {
    _actions_to_process.Insert({ action, _current_time });
  }

  /**
   * Gets a resource
   *
   * @tparam T
   * @param name
   * @param from
   * @return
   */
  template<typename T>
  ResourcePtrT<T> LazyResource(const std::string &name, const source_location &from = source_location::current()) {
    return ResourcePtrT<T>(MakeResourceContainer(name, type_id<T>(), from));
  }
};
}// namespace e00
