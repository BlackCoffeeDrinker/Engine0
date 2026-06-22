
#pragma once

#include <Engine.hpp>

namespace e00::impl {

/**
 * Class PaletteLoader
 */
class PaletteLoader : public ResourceLoader {
public:
  [[nodiscard]] bool SupportsType(type_t type) const override { return type == type_id<FixedPalette>(); }
  [[nodiscard]] bool SupportsOption(type_t optionTypeid) const override { return false; }
  bool CanLoad(const LoadContext& context) override;
  Result ReadLoad(const LoadContext& context) override;
};

}// namespace e00::impl
