#pragma once

#include "Engine.hpp"
#include "Resource/ResourceLoader.hpp"

namespace e00::impl {
class BitmapLoader : public ResourceLoader {
  mutable Logger _logger;

public:
  BitmapLoader();

  ~BitmapLoader() override = default;

  bool CanLoad(Stream &stream) override;

  ResourceLoader::Result ReadLoad(Stream & stream) override;
};
}// namespace e00::impl
