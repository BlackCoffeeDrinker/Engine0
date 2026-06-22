#include "PrivateInclude.hpp"

#include "Platform.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/PaletteLoader.h"
#include "Loaders/PngLoader.hpp"
#include "Loaders/WorldLoader.hpp"

namespace {
constexpr e00::ResourceId default_palette_name = e00::HashName("defaultpalette");
}// namespace

namespace e00 {
std::error_code Init() {
  // Start platform code first
  if (const auto platformInitError = platform::Init()) {
    return platformInitError;
  }
  // Read the game INI
  GetDefaultLogger().Verbose(source_location::current(), "Loading main INI");

  auto &resource_manager = ResourceManager::GlobalResourceManager();

  const auto current_platform = platform::PlatformName();
  const auto platform_prefix = std::string("platform:") + std::string(current_platform);

  if (const auto config = StreamFactory::GlobalStreamFactory().OpenStream("game.ini")) {
    const auto iniEc = impl::IniParser::Parse(*config, [&](const impl::IniParser::Item &item) -> std::error_code {
      if (item.category == "platform" || item.category == platform_prefix) {
        platform::SetSettings(item.key, item.value);
      } else if (item.category == "core") {
        GetDefaultLogger().Info(source_location::current(), "> {} - {} = {}", item.category, item.key, item.value);
      } else if (item.category == "resourcemap") {
        resource_manager.SetAlias(HashName(item.key), item.value);
      }

      return {};
    });

    if (iniEc) {
      GetDefaultLogger().Error(source_location::current(), "Failed to read resource game.ini");
      return impl::make_error_code(impl::EngineErrorCode::bad_configuration_file);
    }
  } else {
    GetDefaultLogger().Error(source_location::current(), "Failed to load resource game.ini");
    return impl::make_error_code(impl::EngineErrorCode::bad_configuration_file);
  }

  // TODO: Add default resource loaders
  std::ignore = resource_manager.AddLoader<impl::BitmapLoader>();
  std::ignore = resource_manager.AddLoader<impl::PNGLoader>();
  std::ignore = resource_manager.AddLoader<impl::WorldLoader>();
  std::ignore = resource_manager.AddLoader<impl::PaletteLoader>();

  return {};
}

void Exit() {
  return platform::Exit();
}

void Run(Engine &engine) {
  GetDefaultLogger().Info(source_location::current(), "Starting up {} with platform {}", engine.Name(), platform::PlatformName());

  // Load a default palette if we find one
  {
    auto &resource_manager = ResourceManager::GlobalResourceManager();
    if (const auto palette = resource_manager.LoadResourceDirectly<FixedPalette>(default_palette_name)) {
      platform::GetMainSurface(engine).SetPalette(palette.Ref());
    }
  }

  // Load up the engine
  if (const auto initError = engine.Init()) {
    GetDefaultLogger().Error(source_location::current(), "Unable to start engine instance for {}: {}", engine.Name(), initError.message());
    return;
  }

  // Do a default title
  platform::SetWindowTitle(engine, engine.Name());

  // Tell the platform to initialize the engine
  if (!platform::InitEngine(engine)) {
    GetDefaultLogger().Error(source_location::current(), "Unable to initialize platform for engine: {}", engine.Name());
    return;
  }
  
  auto last_time = std::chrono::steady_clock::now();
  
  while (engine.IsRunning()) {
    const auto now = std::chrono::steady_clock::now();
    const auto delta = now - last_time;
    last_time = now;

    if (delta > std::chrono::milliseconds(300)) {
      GetDefaultLogger().Info(source_location::current(), "Delta of {} miliseconds!", std::chrono::duration_cast<std::chrono::milliseconds>(delta).count());
    }
    
    // Process platform events
    platform::ProcessEvents(engine);

    // Render
    if (platform::HasFocus(engine)) {
      platform::ProcessDraw(engine);
    } else {
      // If we do not have focus, yield
      platform::Yield();
    }
  }

  platform::QuitEngine(engine);
}
}// namespace e00
