#pragma once

#include <Engine.hpp>

namespace e00::impl {
class GifSpriteLoader : public ResourceLoader {

public:
  ~GifSpriteLoader() override;
  [[nodiscard]] bool SupportsOption(type_t optionTypeid) const override;
  bool SupportsType(type_t type) const override { return type == type_id<Sprite>(); }
  bool CanLoad(const LoadContext& context) override;
  Result ReadLoad(const LoadContext& context) override;
};
}// namespace e00::impl
