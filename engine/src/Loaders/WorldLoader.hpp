#pragma once

#include <Engine.hpp>

namespace e00::impl {

/**
 * This is a *Map* loader, not a world loader
 * 
 */
class WorldLoader : public ResourceLoader {
  std::error_code ParseTileset(Stream &stream, const std::unique_ptr<Map> &map);
  std::error_code ParseSet(Stream &stream, const std::unique_ptr<Map> &map);

public:
  WorldLoader();
  ~WorldLoader() override;

  [[nodiscard]] bool SupportsOption(type_t optionTypeid) const override;
  bool SupportsType(type_t type) const override { return type == type_id<Map>(); }
  bool CanLoad(const LoadContext& context) override;
  Result ReadLoad(const LoadContext& context) override;
};
}// namespace e00::impl
