#pragma once

#include <Engine.hpp>

namespace e00::impl {
class BitmapLoader : public ResourceLoader {
public:
  BitmapLoader();

  ~BitmapLoader() override = default;

  bool SupportsType(type_t type) const override { return type == type_id<Bitmap>(); }

  bool CanLoad(const LoadContext& context) override;

  Result ReadLoad(const LoadContext& context) override;
};
}// namespace e00::impl
