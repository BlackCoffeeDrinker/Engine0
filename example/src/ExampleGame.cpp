
#include "ExampleGame.hpp"

ExampleGame::ExampleGame() : Engine() {
}

ExampleGame::~ExampleGame() {
  e00::GetDefaultLogger().Info(e00::source_location::current(), "~ExampleGame");
}

std::string_view ExampleGame::Name() const noexcept {
  return "Example Bad Game";
}

std::error_code ExampleGame::OnInit() {


  return LoadWorld("helloworld");
}

void ExampleGame::OnFirstTick() {
  e00::GetDefaultLogger().Info(e00::source_location::current(), "First Tick");
}

void ExampleGame::OnPause() {
  e00::GetDefaultLogger().Info(e00::source_location::current(), "On Pause");
}

void ExampleGame::OnResume() {
  e00::GetDefaultLogger().Info(e00::source_location::current(), "On Resume");
}
void ExampleGame::OnWorldUnload(const std::unique_ptr<e00::World> &old_world) {
  if (_world_widget) {
    _world_widget->Parent()->RemoveChild(_world_widget);//discard
    _world_widget = nullptr;
  }
}
void ExampleGame::OnWorldLoaded(const std::unique_ptr<e00::World> &new_world) {
  if (_world_widget) {
    _world_widget->Parent()->RemoveChild(_world_widget);//discard
  }

  _world_widget = RootWidget()->AddChild<e00::WorldWidget>(new_world);
  _world_widget->SetFixedSize({640, 480 - 80});
}

void ExampleGame::ExecuteAction(const e00::Action &action) {
  Engine::ExecuteAction(action);
}
