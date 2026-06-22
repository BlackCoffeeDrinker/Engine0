#pragma once

#include <Engine.hpp>

namespace e00::impl {

/**
 * Class PNGLoader
 */
class PNGLoader : public ResourceLoader {
public:
  bool SupportsType(type_t type) const override { return type == type_id<Bitmap>(); }
  [[nodiscard]] bool SupportsOption(type_t optionTypeid) const override;
  bool CanLoad(const LoadContext& context) override;
  Result ReadLoad(const LoadContext& context) override;
};

}// namespace e00::impl
