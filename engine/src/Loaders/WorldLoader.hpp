#pragma once

#include "Resource/ResourceLoader.hpp"
#include <Engine.hpp>

namespace e00::impl {
class WorldLoader : public ResourceLoader {
  mutable Logger _logger;

  // Hide the actual implementation, because it adds too much compilation time otherwise
  class Impl;
  std::unique_ptr<Impl> _impl;

public:
  WorldLoader();

  ~WorldLoader() override;

  bool SupportsType(type_t type) const override {
    return type == type_id<World>();
  }

  bool CanLoad(Stream &stream) override;

  ResourceLoader::Result ReadLoad(Stream &stream) override;
};
}// namespace e00::impl
