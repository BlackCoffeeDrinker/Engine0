#pragma once

#include <Engine.hpp>

namespace e00::impl {

/**
 * Class TilesetLoader
 */
class TilesetLoader : public ResourceLoader {
  std::map<TileIdType, std::string> _tileProperties;
  size_t _margin {0};
  size_t _spacing {0};
  size_t _tileWidth {0};
  size_t _tileHeight {0};
  ResourcePtrT<Bitmap> _sourceAtlas;

  std::error_code HandleTilesetSection(std::string_view key, std::string_view value);
  std::error_code HandleTileSpecificSection(TileIdType tileId, std::string_view key, std::string_view value);

public:
  [[nodiscard]] bool SupportsType(type_t type) const override { return type == type_id<Tileset>(); }
  bool CanLoad(const LoadContext &context) override;
  Result ReadLoad(const LoadContext &context) override;
};

}// namespace e00::impl
