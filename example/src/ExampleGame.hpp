
#pragma once

#include <Engine.hpp>

class ExampleGame : public e00::Engine {
  e00::WorldWidget *_world_widget = nullptr;

protected:
  void ExecuteAction(const e00::Action &action) override;

  std::error_code OnInit() override;
  void OnFirstTick() override;
  void OnPause() override;
  void OnResume() override;
  void OnWorldUnload(const std::unique_ptr<e00::World> &old_world) override;
  void OnWorldLoaded(const std::unique_ptr<e00::World> &new_world) override;

public:
  ExampleGame();
  ~ExampleGame() override;

  [[nodiscard]] std::string_view Name() const noexcept override;
};
