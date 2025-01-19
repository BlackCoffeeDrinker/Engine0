#pragma once

#include "PrivateInclude.hpp"
#include <queue>

namespace e00 {
enum class EngineState {
  FIRST_TICK,
  RUNNING_NORMAL,
  PAUSE,
  QUIT
};

class Engine::Data {
  Engine *const _public_engine;

  Logger _main_logger;
  EngineState _state;
  EngineState _old_state;

  std::priority_queue<ActionInstance> _actions_to_execute;//< Actions in queue
  std::unique_ptr<ScriptEngine> _script_engine;           //< Persistent script engine
  std::unique_ptr<World> _current_world;                  //< Currently main world
  std::unique_ptr<Widget> _root_widget;                   //< Root widget where we draw from

  void ProcessAction(const Action &action);

public:
  explicit Data(Engine *publicInterface);

  ~Data() = default;

  [[nodiscard]] auto State() const { return _state; }

  [[nodiscard]] auto OldState() const { return _old_state; }

  void ClearOldState() { _old_state = _state; }

  void SetState(const EngineState engineState) {
    _old_state = _state;
    _state = engineState;
  }

  void TickWorld(const std::chrono::milliseconds &delta) {
    if (_current_world) {
      // TODO
    }
  }

  void QueueAction(Action action, GameClock::time_point tp) {
    _actions_to_execute.emplace(action, tp);
  }

  void ExecuteActionsAtTime(const GameClock::time_point &tp) {
    // Drain the list until time_point tp
    while (!_actions_to_execute.empty() && _actions_to_execute.top().when <= tp) {
      const auto &topAction = _actions_to_execute.top();

      // Is the root widget interested in this action?
      const auto widgetProcessResult = RootWidget().ProcessAction(topAction);
      if (widgetProcessResult != Widget::ActionProcessResult::HandledAndConsumed) {
        if (_current_world) {
          _current_world->ProcessAction(topAction);
        }

        ProcessAction(topAction.action);
      }

      _actions_to_execute.pop();
    }
  }

  Widget &RootWidget();
};

}// namespace e00
