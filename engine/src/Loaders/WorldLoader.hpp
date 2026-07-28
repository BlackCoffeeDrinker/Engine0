#pragma once

#include <Engine.hpp>

namespace e00::impl {

/**
 * This is a *Map* loader, not a world loader
 * 
 */
class WorldLoader : public ResourceLoader {
  struct CurrentLoadContext {
    size_t width = 0;
    size_t height = 0;
    std::unique_ptr<Map> map;
  };

  std::error_code ParseTileset(Stream &stream, const std::unique_ptr<Map> &map);
  std::error_code ParseSet(Stream &stream, const std::unique_ptr<Map> &map);

  std::error_code HandleWorldData(std::string_view category, std::string_view key,
                                  std::string_view value);

  std::error_code HandleMapData(std::string_view key, std::string_view value);
  std::error_code HandleTilesetData(std::string_view key, std::string_view value);
  std::error_code HandleTileData(std::string_view tileId, std::string_view key, std::string_view value);
  std::error_code HandleTileTypeConfigData(std::string_view tileType, std::string_view key, std::string_view value);

  CurrentLoadContext currentLoadContext;

public:
  WorldLoader();
  ~WorldLoader() override;

  [[nodiscard]] bool SupportsOption(type_t optionTypeid) const override;
  [[nodiscard]] bool SupportsType(type_t type) const override { return type == type_id<Map>(); }
  bool CanLoad(const LoadContext &context) override;
  Result ReadLoad(const LoadContext &context) override;
};
}// namespace e00::impl
