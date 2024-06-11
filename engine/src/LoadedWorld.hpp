#pragma once

#include <Engine.hpp>


namespace e00 {
class LoadedWorld {
  Logger _log;

  std::unique_ptr<World> _world;
  Vec2D<uint16_t> _world_size_tiles;//< Display size, in tiles
  Vec2D<uint16_t> _camera_center;   //< Where we're looking at

public:
  NOT_COPYABLE(LoadedWorld);

  LoadedWorld(const std::string &worldName, ResourcePtrT<Map> &&map);

  ~LoadedWorld();

  std::error_code Init();

  void Tick(const std::chrono::milliseconds &delta) noexcept;

  void Draw(Bitmap &bitmap) noexcept;
};
}// namespace e00
