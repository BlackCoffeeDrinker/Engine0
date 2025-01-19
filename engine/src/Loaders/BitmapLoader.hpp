#pragma once

#include <Engine.hpp>

namespace e00::impl {
class BitmapLoader : public ResourceLoader {
  mutable Logger _logger;

public:
  BitmapLoader();

  ~BitmapLoader() override = default;

  bool SupportsType(type_t type) const override { return type == type_id<Bitmap>(); }

  bool CanLoad(Stream &stream) override;

  Result ReadLoad(Stream &stream) override;
};
}// namespace e00::impl
