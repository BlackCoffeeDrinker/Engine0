#pragma once

/*
 * Includes are here to keep the static analysis tool happy
 */
#include <chrono>
#include <map>
#include <memory>
#include <system_error>

#include "Action.hpp"
#include "InputEvent.hpp"

namespace e00 {
/**
 * Root engine class
 */
class Engine {
  friend class WorldWidget;
  class Data;
  
  Logger _main_logger;

  GameClock::time_point _current_game_time;//< Current game time
  std::map<InputEvent, Action> _input_binding;//< Input event to actions
  std::unique_ptr<Data> _pImpl;                //< Impl data

protected:
  explicit Engine();

  /**
   * Execute an action
   *
   * @param action the action to execute
   */
  virtual void ExecuteAction(const Action &action);

  /**
   * Perform INIT code (maybe load the main menu, fonts, ...)
   * @return any error
   */
  virtual std::error_code OnInit() { return {}; }

  /**
   * Executed on first tick
   */
  virtual void OnFirstTick() {}

  /**
   * Called when the engine is set into pause
   */
  virtual void OnPause() {}

  /**
   * Called when the engine is resumed from pause
   */
  virtual void OnResume() {}

public:
  NOT_COPYABLE(Engine);

  /**
   *
   * @return the built in "Quit" action
   */
  static Action BuiltInAction_Quit();

  /**
   *
   * @return the build in "Toggle Pause" action
   */
  static Action BuiltInAction_PauseToggle();

  /**
   *
   */
  virtual ~Engine();

  /**
   * Called to initialize this instance
   *
   * @return any error code
   */
  std::error_code Init() noexcept;

  /**
   * Sets this instance language code for text
   *
   * @param languageCode "en", "fr"
   * @return true if a valid language code was passed
   */
  bool LanguageCode(const std::string &languageCode);

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
   * Queries if this engine instance is in pause
   *
   * @return true if this engine instance is paused
   */
  [[nodiscard]] bool IsPaused() const noexcept;

  /**
   * Game time
   * @return current game time
   */
  [[nodiscard]] GameClock::time_point Now() const noexcept { return _current_game_time; }

  /**
   * Processes a delta tick
   *
   * @param delta time since last tick
   */
  void Tick(const std::chrono::milliseconds &delta) noexcept;

  /**
   * Informs this instance's engine that an Input has been received
   *
   * @param event the event
   */
  void ProcessInputEvent(const InputEvent event) {
    if (const auto &event_binding = _input_binding.find(event);
        event_binding != _input_binding.end()) {
      QueueActionForNextTick(event_binding->second);
    }
  }

  /**
   * Unconditionally execute this action on the next tick
   *
   * @param action the action
   */
  void QueueActionForNextTick(Action action);

  /**
   * 
   * @return reference to the root widget
   */
  Widget & RootWidget();

  /**
   * Gets a resource
   *
   * @tparam T
   * @param name
   * @param from
   * @return
   */
  template<typename T>
  ResourcePtrT<T>
  LazyResource(const std::string &name,
               const source_location &from = source_location::current()) {
    return ResourceManager::GlobalResourceManager().LazyResource<T>(name, from);
  }
};
}// namespace e00
