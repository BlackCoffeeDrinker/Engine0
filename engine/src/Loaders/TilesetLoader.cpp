
#include "TilesetLoader.hpp"

#include "IniParser.hpp"


namespace {
constexpr std::string_view kTileIdPrefix = "id:";
}// namespace

namespace e00::impl {
std::error_code TilesetLoader::HandleTilesetSection(std::string_view key, std::string_view value) {
  if (key == "source") {
    _sourceAtlas = _engine->LazyResource<Bitmap>(HashName(value));
  }

  if (key == "tilewidth") {
    if (const auto size_ec = ToSize<uint16_t>(value, _tileWidth)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse tilewidth {}", value);
      return size_ec;
    }
  }

  if (key == "tileheight") {
    if (const auto size_ec = ToSize<uint16_t>(value, _tileHeight)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse tileheight {}", value);
      return size_ec;
    }
  }

  if (key == "spacing") {
    if (const auto size_ec = ToSize<uint16_t>(value, _spacing)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse spacing {}", value);
      return size_ec;
    }
  }

  if (key == "margin") {
    if (const auto size_ec = ToSize<uint16_t>(value, _margin)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse margin {}", value);
      return size_ec;
    }
  }

  return {};
}

std::error_code TilesetLoader::HandleTileSpecificSection(TileIdType tileId, std::string_view key, std::string_view value) {
  if (!_tileProperties.contains(tileId)) {
    _tileProperties[tileId] = {};
  }

  return {};
}

ResourceLoader::Result TilesetLoader::ReadLoad(const LoadContext &context) {
  const auto ec = IniParser::Parse(context.stream, [&](const IniParser::Item &item) -> std::error_code {
    if (item.category == "tileset") {
      return HandleTilesetSection(item.key, item.value);
    }

    if (item.category.starts_with(kTileIdPrefix)) {
      const auto tile_number_str = item.category.substr(kTileIdPrefix.size());
      size_t tile_number;
      if (const auto size_ec = ToSize<TileIdType>(tile_number_str, tile_number)) {
        GetDefaultLogger().Error(source_location::current(), "Failed to parse tile number {}", tile_number_str);
        return size_ec;
      }

      return HandleTileSpecificSection(tile_number, item.key, item.value);
    }

    return {};
  });

  if (ec) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse tileset: {}", ec.message());
    return ec;
  }

  auto ret = std::make_unique<Tileset>();
  ret->SetTileset(std::move(_sourceAtlas));
  ret->SetTileSize({static_cast<BitmapSizeType>(_tileWidth), static_cast<BitmapSizeType>(_tileHeight)});
  ret->SetMargin(static_cast<BitmapSizeType>(_margin));
  ret->SetSpacing(static_cast<BitmapSizeType>(_spacing));

  return ret;
}

bool TilesetLoader::CanLoad(const LoadContext &context) {
  return true;
}
}// namespace e00::impl
