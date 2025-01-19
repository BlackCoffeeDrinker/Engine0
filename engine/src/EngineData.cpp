
#include "EngineData.hpp"

namespace e00 {
Engine::Data::Data(Engine *publicInterface) : _public_engine(publicInterface),
                                              _main_logger(CreateSink("EnginePrivate")),
                                              _state(EngineState::FIRST_TICK),
                                              _old_state(_state),
                                              _script_engine(ScriptEngine::Create()),
                                              _root_widget(std::make_unique<Widget>()) {

  _main_logger.Verbose(source_location::current(), "E0 Starting");
}

Widget &Engine::Data::RootWidget() {
  return *_root_widget;
}

void Engine::Data::ProcessAction(const Action &action) {
  if (action == BuiltInAction_Quit()) {
    SetState(EngineState::QUIT);
  } else if (action == BuiltInAction_PauseToggle()) {
    if (State() == EngineState::PAUSE) {
      SetState(EngineState::RUNNING_NORMAL);
    } else {
      SetState(EngineState::PAUSE);
    }
  }
}

}// namespace e00
