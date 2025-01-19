
#include "ExampleGame.hpp"

ExampleGame::ExampleGame() : Engine(),
                             _game_logger(e00::CreateSink("ExampleGame")) {
}

ExampleGame::~ExampleGame() {
  _game_logger.Info(e00::source_location::current(), "~ExampleGame");
}

std::string_view ExampleGame::Name() const noexcept {
  return "Example Bad Game";
}

std::error_code ExampleGame::OnInit() {

  return {};
}

void ExampleGame::OnFirstTick() {
  _game_logger.Info(e00::source_location::current(), "First Tick");
}

void ExampleGame::OnPause() {
  _game_logger.Info(e00::source_location::current(), "On Pause");
}

void ExampleGame::OnResume() {
  _game_logger.Info(e00::source_location::current(), "On Resume");
}

void ExampleGame::ExecuteAction(const e00::Action &action) {
  Engine::ExecuteAction(action);
}
