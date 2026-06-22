#pragma once

/*
 * Includes are here to keep the static analysis tool happy
 */
#include <chrono>
#include <map>
#include <memory>
#include <queue>
#include <system_error>

namespace e00 {

class TranslatableText;
class PlatformData;

/**
 * Root engine class
 */
class Engine {
  friend class WorldWidget;

  enum class EngineState {
    FIRST_TICK,
    RUNNING_NORMAL,
    PAUSE,
    QUIT
  };

  GameClock::time_point _current_game_time;   //< Current game time
  std::map<InputEvent, Action> _input_binding;//< Input event to actions

  EngineState _state;    // Current engine state
  EngineState _old_state;// Previous engine state, previous tick

  std::string _current_locale;

  std::priority_queue<ActionInstance> _actions_to_execute;//< Actions in queue
  std::unique_ptr<ScriptEngine> _script_engine;           //< Persistent script engine
  std::unique_ptr<World> _current_world;                  //< Currently main world
  std::unique_ptr<Widget> _root_widget;                   //< Root widget where we draw from
  std::unique_ptr<TranslatableText> _strings;             //< Strings dictionary

  PlatformData *_platform_data;// << Opaque data associated with this instance, platform is responsible for managing it

  void ExecuteActionsAtTime(const GameClock::time_point &tp);

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

  virtual void OnWorldUnload(const std::unique_ptr<World> &old_world) {}

  /**
   * 
   * @param new_world the new world that was loaded
   */
  virtual void OnWorldLoaded(const std::unique_ptr<World> &new_world) {}

  /**
   * 
   * @param locale the locale of the text to add
   * @param textCode
   * @param text the text
   * @return any errors
   */
  std::error_code AddText(const std::string &locale, int textCode, const std::string &text);


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
   * Load a new world
   * 
   * @param world_name the ressource name
   * @return any errors
   */
  std::error_code LoadWorld(const std::string &world_name);

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
  void ProcessInputEvent(InputEvent event);

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
  Widget *RootWidget();
  
  // All platform
  [[nodiscard]] auto GetPlatformData() const noexcept { return _platform_data; }
  void SetPlatformData(PlatformData *data) noexcept { _platform_data = data; }

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
