#include "PrivateInclude.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/WorldLoader.hpp"

namespace {
// TODO: Put this in Engine class for each game can have it's optimal frame rate
const auto OPTIMAL_RENDER_DELAY = std::chrono::milliseconds(32);


std::unique_ptr<e00::ResourceManager> glb_resource_manager;//< Global resource manager
}// namespace

namespace e00 {
std::error_code Init() {
  Logger rootLogger(CreateSink("Init"));

  // Start platform code first
  if (const auto platformInitError = platform::Init()) {
    return platformInitError;
  }

  // Read the game INI
  rootLogger.Verbose(source_location::current(), "Loading main INI");

  if (auto config = platform::OpenStream("game.ini")) {
    const auto iniEc = impl::IniParser::Parse(*config, [&](const impl::IniParser::Item &item) -> std::error_code {
      rootLogger.Info(source_location::current(), "> {} - {} = {}", item.category, item.key, item.value);
      return {};
    });

    if (iniEc) {
      rootLogger.Error(source_location::current(), "Failed to load resource game.ini");
      return impl::make_error_code(impl::EngineErrorCode::bad_configuration_file);
    }
  } else {
    rootLogger.Error(source_location::current(), "Failed to load resource game.ini");
    return impl::make_error_code(impl::EngineErrorCode::bad_configuration_file);
  }

  // Create the global resource manager
  glb_resource_manager = std::make_unique<ResourceManager>();

  // TODO: Add default resource loaders
  glb_resource_manager->AddLoader(std::make_unique<impl::BitmapLoader>());
  glb_resource_manager->AddLoader(std::make_unique<impl::WorldLoader>());

  return {};
}

void Exit() {
  glb_resource_manager.reset();
  return platform::Exit();
}

const std::unique_ptr<ResourceManager> &GlobalResourceManager() {
  return glb_resource_manager;
}

void Run(Engine &engine) {
  Logger rootLogger(CreateSink("Loop"));
  rootLogger.Info(source_location::current(), "Starting up {} with platform {}", engine.Name(), platform::PlatformName());

  // Load up the engine
  if (const auto initError = engine.Init()) {
    rootLogger.Error(source_location::current(), "Unable to start engine instance for {}: {}", engine.Name(), initError.message());
    return;
  }

  auto oldTime = std::chrono::steady_clock::now();
  auto lastRenderTime = std::chrono::steady_clock::now();

  while (engine.IsRunning()) {
    auto nowPoint = std::chrono::steady_clock::now();
    const auto endPoint = nowPoint + OPTIMAL_RENDER_DELAY;

    // Compute delta + tick
    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(nowPoint - oldTime);

    // Tick every system (Maybe make a SubSystem class with Init/Tick/Deinit ?)
    engine.Tick(delta);
    glb_resource_manager->Tick(delta);

    // Update old time, so we can compute delta next round
    // We probably should compute how long those ticks took to make sure we aren't going above
    oldTime = nowPoint;

    // Process platform events
    while (std::chrono::steady_clock::now() < endPoint) {
      platform::ProcessEvents(engine);
    }

    // Render
    if (platform::HasFocus()) {
      // make sure we have the optimal tick rate
      if (std::chrono::duration_cast<std::remove_cv_t<decltype(OPTIMAL_RENDER_DELAY)>>(nowPoint - lastRenderTime) > OPTIMAL_RENDER_DELAY) {
        engine.Draw();
        platform::ProcessDraw(engine);
      }
    } else {
      // If we don't have focus, yield
      platform::Yield();
    }
  }
}
}// namespace e00