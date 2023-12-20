#pragma once

#include "Engine.hpp"
#include "Resource/ResourceLoader.hpp"

namespace e00::impl {
class WorldLoader : public ResourceLoader {
  mutable Logger _logger;

  // Hide the actual implementation, because it adds too much compilation time otherwise
  class Impl;
  std::unique_ptr<Impl> _impl;

public:
  WorldLoader();

  ~WorldLoader() override;

  bool CanLoad(Stream &stream) override;

  ResourceLoader::Result ReadLoad(Stream& stream) override;
};
}// namespace e00::impl
