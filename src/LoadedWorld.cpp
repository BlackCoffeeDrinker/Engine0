#include "LoadedWorld.hpp"

namespace e00 {

Engine::LoadedWorld::LoadedWorld(const std::string &worldName, ResourcePtrT<Map> &&map)
  : _log(CreateSink("LoadedWorld")) {
  _log.Info(source_location::current(), "Creating instance of {}", worldName);
  _world = std::make_unique<World>(worldName, std::move(map));
}

Engine::LoadedWorld::~LoadedWorld() = default;

std::error_code Engine::LoadedWorld::Init() {
  // Compute the ## of tiles in width, height for the display
  _world_size_tiles.x = 0;
  _world_size_tiles.y = 0;


  return {};
}

void Engine::LoadedWorld::Tick(const std::chrono::milliseconds &delta) noexcept {
  // Process all actors
  for (const auto &actor : _world->Actors()) {
  }
}

void Engine::LoadedWorld::Draw(Bitmap &bitmap) noexcept {
  // Compute the ## of tiles needed
  if (_world_size_tiles.x == 0 || _world_size_tiles.y == 0) {
    _world_size_tiles = bitmap.Size() / _world->Map()->Tileset().TileSize();
  }

  // Make a window where the "camera" is
  const auto w = _world->MakeWindow({}, _world_size_tiles);

  for (uint16_t y = 0; y < w.Height(); y++) {
    for (uint16_t x = 0; x < w.Width(); x++) {
      _world->Map()->Tileset().DrawTile(0, bitmap, {});
    }
  }
}
}// namespace e00
