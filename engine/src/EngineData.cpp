
#include "EngineData.hpp"

namespace e00 {
Engine::Data::Data() : _main_logger(CreateSink("Engine")),
                       _state(EngineState::INIT),
                       _script_engine(ScriptEngine::Create()) {

  _main_logger.Verbose(source_location::current(), "E0 Starting");
}

Engine::Data::~Data() {

}

std::error_code Engine::Data::Init() {
  // Make sure we're in the right state
  if (_state != EngineState::INIT) {
    _main_logger.Error(source_location::current(), "Trying to re-initialize engine");
    return {};
  }

  // Set next state
  _state = EngineState::FIRST_TICK;
  return {};
}

void Engine::Data::Tick(const std::chrono::milliseconds &delta) noexcept {
  // Update the current world
  if (_current_world) {
    _current_world->Tick(delta);
  }
}

void Engine::Data::Draw() {

}

void Engine::Data::ExecuteAction(Action action) {

}

}// namespace e00