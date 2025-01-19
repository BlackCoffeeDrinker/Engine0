#include "PrivateInclude.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/WorldLoader.hpp"

namespace {
// TODO: Put this in Engine class for each game can have it's optimal frame rate
constexpr auto OPTIMAL_RENDER_DELAY = std::chrono::milliseconds(32);
}// namespace

namespace e00 {
std::error_code Init() {
  const Logger rootLogger(CreateSink("Init"));

  // Start platform code first
  if (const auto platformInitError = platform::Init()) {
    return platformInitError;
  }

  // Read the game INI
  rootLogger.Verbose(source_location::current(), "Loading main INI");

  if (const auto config = platform::OpenStream("game.ini")) {
    const auto iniEc = impl::IniParser::Parse(*config, [&](const impl::IniParser::Item &item) -> std::error_code {
      if (item.category == "platform") {
        platform::SetSettings(item.key, item.value);
      } else if (item.category == "core") {
        rootLogger.Info(source_location::current(), "> {} - {} = {}", item.category, item.key, item.value);
      }

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

  // TODO: Add default resource loaders
  (void) ResourceManager::GlobalResourceManager().AddLoader<impl::BitmapLoader>();
  (void) ResourceManager::GlobalResourceManager().AddLoader<impl::WorldLoader>();

  return {};
}

void Exit() {
  return platform::Exit();
}

void Run(Engine &engine) {
  const Logger rootLogger(CreateSink("Loop"));
  rootLogger.Info(source_location::current(), "Starting up {} with platform {}", engine.Name(), platform::PlatformName());

  // Load up the engine
  if (const auto initError = engine.Init()) {
    rootLogger.Error(source_location::current(), "Unable to start engine instance for {}: {}", engine.Name(), initError.message());
    return;
  }

  // Do a default title
  platform::SetWindowTitle(engine.Name());

  auto oldTime = std::chrono::steady_clock::now();
  auto lastRenderTime = std::chrono::steady_clock::now();

  while (engine.IsRunning()) {
    auto nowPoint = std::chrono::steady_clock::now();
    const auto endPoint = nowPoint + OPTIMAL_RENDER_DELAY;

    // Compute delta + tick
    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(nowPoint - oldTime);

    // Tick every system (Maybe make a SubSystem class with Init/Tick/Deinit ?)
    engine.Tick(delta);
    // TODO: ? ResourceManager::GlobalResourceManager().Tick(delta);

    // Update old time, so we can compute delta next round
    // We probably should compute how long those ticks took to make sure we aren't going above
    oldTime = nowPoint;

    // Process platform events (make sure we get atleast one loop)
    do {
      platform::ProcessEvents(engine);
    } while (std::chrono::steady_clock::now() < endPoint);

    // Render
    if (platform::HasFocus()) {
      // make sure we have the optimal tick rate
      if (std::chrono::duration_cast<std::remove_cv_t<decltype(OPTIMAL_RENDER_DELAY)>>(nowPoint - lastRenderTime) > OPTIMAL_RENDER_DELAY) {
        platform::ProcessDraw(engine);
        lastRenderTime = nowPoint;
      }
    } else {
      // If we do not have focus, yield
      platform::Yield();
    }
  }
}
}// namespace e00
