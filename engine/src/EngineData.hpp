
#pragma once

#include "LoadedWorld.hpp"
#include "PrivateInclude.hpp"

namespace e00 {
class ResourceManager;

enum class EngineState {
  INIT,
  FIRST_TICK,
  RUNNING_NORMAL,
  PAUSE,

  QUIT
};

class Engine::Data {
  Logger _main_logger;

  EngineState _state;

  detail::CircularBuffer<ActionInstance, 254> _actions_to_process;//< Actions to be executed next tick

  std::unique_ptr<ScriptEngine> _script_engine;                   //< Persistent script engine
  std::unique_ptr<LoadedWorld> _current_world;                    //< Currently main world

public:
  Data();
  ~Data();

  [[nodiscard]] auto State() const { return _state; }

  void SetState(EngineState engineState) { _state = engineState; }

  void ExecuteAction(Action action);

  std::error_code Init();

  void Tick(const std::chrono::milliseconds &delta) noexcept;

  void Draw();

};

}// namespace e00
