
#pragma once

#include <Engine.hpp>

class ExampleGame : public e00::Engine {
  e00::Logger _game_logger;

protected:
  void ExecuteAction(const e00::Action &action) override;

  std::error_code OnInit() override;
  void OnFirstTick() override;
  void OnPause() override;
  void OnResume() override;

public:
  ExampleGame();
  ~ExampleGame() override;

  [[nodiscard]] std::string_view Name() const noexcept override;
};
