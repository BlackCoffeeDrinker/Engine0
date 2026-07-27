#include "WorldLoader.hpp"
#include "EngineError.hpp"
#include "IniParser.hpp"

#include <charconv>

namespace {
constexpr std::string_view kTilePrefix = "tile:";
constexpr std::string_view kTileTypeConfigPrefix = "type:";

template<typename RealType = e00::WorldCoordinateType>
std::error_code ToSize(const std::string_view &str, size_t &size) {
  if (const auto value = std::from_chars(str.data(), str.data() + str.size(), size);
      value.ec != std::errc{}) {
    return std::make_error_code(value.ec);
  }

  if (size > std::numeric_limits<RealType>::max()) {
    return std::make_error_code(std::errc::invalid_argument);
  }
  if (size < std::numeric_limits<RealType>::min()) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  return {};
}

}// namespace

namespace e00::impl {

WorldLoader::WorldLoader()
    : ResourceLoader() {}

WorldLoader::~WorldLoader() = default;
bool WorldLoader::SupportsOption(type_t optionTypeid) const {
  return type_id<DiscardPalette>() == optionTypeid;
}

std::error_code WorldLoader::ParseTileset(Stream &stream, const std::unique_ptr<Map> &map) {
  return IniParser::Parse(stream, [&](const IniParser::Item &item) -> std::error_code {
    if (item.category == "image") {
    }

    if (item.category == "tiles") {
    }

    if (item.category.starts_with("tile:")) {
      // Per tile options
      GetDefaultLogger().Info(source_location::current(), "Tile: {} -> {} = {}", item.category, item.key, item.value);
    }

    return {};
  });
}

std::error_code WorldLoader::ParseSet(Stream &stream, size_t layerIndex, const std::unique_ptr<Map> &map) {
  if (const auto ec = stream.SeekTo(0)) {
    return ec;
  }

  Position idx(0, 0);
  int current = -1;

  while (!stream.AtEnd()) {
    char c;
    if (const auto ec = stream.Read(c)) {
      return ec;
    }

    if (c >= '0' && c <= '9') {
      if (current == -1) {
        current = 0;
      }

      current = current * 10 + (c - '0');
    } else if (c == ',') {
      map->Set(layerIndex, idx, current);

      idx.x++;

      if (idx.x >= map->Size().x) {
        idx.x = 0;
        idx.y++;
      }
      current = -1;
    } else if (current != -1) {
      // Ignore invalid characters
    }
  }

  if (current != -1) {
    map->Set(layerIndex, idx, current);
  }

  return {};
}

std::error_code WorldLoader::HandleMapData(std::string_view key, std::string_view value) {
  if (key == "layers") {
    size_t layerCount = 0;
    if (const auto size_ec = ToSize(value, layerCount)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse width {}", value);
      return size_ec;
    }

    currentLoadContext.map->SetLayerCount(static_cast<uint16_t>(layerCount));
    return {};
  }

  return {};
}

std::error_code WorldLoader::HandleLayerData(std::string_view key, std::string_view value) {
  // Key is layer, value is data file name
  size_t layerIndex;
  if (const auto size_ec = ToSize(key, layerIndex)) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse layer index {}", key);
    return size_ec;
  }

  if (layerIndex >= currentLoadContext.map->GetLayerCount()) {
    GetDefaultLogger().Error(source_location::current(), "Layer index {} out of range", layerIndex);
    return std::make_error_code(std::errc::invalid_argument);
  }

  if (const auto &set = _engine->FindStreamForResource(HashName(value))) {
    return ParseSet(*set, layerIndex, currentLoadContext.map);
  }

  return {};
}

std::error_code WorldLoader::HandleTilesetData(std::string_view key, std::string_view value) {
  if (key == "source") {
    currentLoadContext.map->SetTileset(_engine->LazyResource<Bitmap>(HashName(value)));
    return {};
  }

  if (key == "tilewidth") {
    size_t tileWidth;
    if (const auto size_ec = ToSize(value, tileWidth)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse tilewidth {}", value);
      return size_ec;
    }
    auto tileSize = currentLoadContext.map->TileSize();
    tileSize.x = tileWidth;
    currentLoadContext.map->SetTileSize(tileSize);
  }

  if (key == "tileheight") {
    size_t tileHeight;
    if (const auto size_ec = ToSize(value, tileHeight)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse tileheight {}", value);
      return size_ec;
    }
    auto tileSize = currentLoadContext.map->TileSize();
    tileSize.y = tileHeight;
    currentLoadContext.map->SetTileSize(tileSize);
  }

  if (key == "spacing") {
    size_t spacing;
    if (const auto size_ec = ToSize(value, spacing)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse spacing {}", value);
      return size_ec;
    }
    currentLoadContext.map->SetTilesetSpacing(spacing);
  }

  if (key == "margin") {
    size_t margin;
    if (const auto size_ec = ToSize(value, margin)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse margin {}", value);
      return size_ec;
    }
    //currentLoadContext.map->SetTilesetMargin(margin);
  }

  return {};
}

std::error_code WorldLoader::HandleTileData(std::string_view tileId, std::string_view key, std::string_view value) {
  GetDefaultLogger().Info(source_location::current(), "Tile {} data key {}, value {}", tileId, key, value);
  return {};
}

std::error_code WorldLoader::HandleWorldData(std::string_view category, std::string_view key, std::string_view value) {
  // If we don't have a map yet, we should only allow map category, with width & height
  if (!currentLoadContext.map) {
    if (category != "map") {
      GetDefaultLogger().Error(source_location::current(), "World is not valid, \"map\" needs to be the first category");
      return std::make_error_code(std::errc::invalid_argument);
    }

    if (key == "width") {
      if (const auto size_ec = ToSize(value, currentLoadContext.width)) {
        GetDefaultLogger().Error(source_location::current(), "Failed to parse width {}", value);
        return size_ec;
      }
    } else if (key == "height") {
      if (const auto size_ec = ToSize(value, currentLoadContext.height)) {
        GetDefaultLogger().Error(source_location::current(), "Failed to parse height {}", value);
        return size_ec;
      }
    } else {
      GetDefaultLogger().Error(source_location::current(), R"(World is not valid: "width" or "height" needs to be the first keys in "map" category)");
      return std::make_error_code(std::errc::invalid_argument);
    }

    if (currentLoadContext.height != 0 && currentLoadContext.width != 0) {
      currentLoadContext.map = std::make_unique<Map>(currentLoadContext.width, currentLoadContext.height);
      currentLoadContext.height = 0;
      currentLoadContext.width = 0;
    }

    return {};
  }

  // We have a map here!
  if (category == "map") return HandleMapData(key, value);
  if (category == "tileset") return HandleTilesetData(key, value);
  if (category == "layers") return HandleLayerData(key, value);
  if (category.starts_with(kTilePrefix)) return HandleTileData(category.substr(kTilePrefix.size()), key, value);
  if (category.starts_with(kTileTypeConfigPrefix)) return HandleTileTypeConfigData(category.substr(kTileTypeConfigPrefix.size()), key, value);

  return {};
}

std::error_code WorldLoader::HandleTileTypeConfigData(std::string_view tileType, std::string_view key, std::string_view value) {
  GetDefaultLogger().Info(source_location::current(), "Tile type config: {} {} {}", tileType, key, value);
  return {};
}

bool WorldLoader::CanLoad(const LoadContext &context) {
  return true;
}

ResourceLoader::Result WorldLoader::ReadLoad(const LoadContext &context) {
  currentLoadContext = {};

  const auto ec = IniParser::Parse(context.stream, [&](const IniParser::Item &item) -> std::error_code {
    return this->HandleWorldData(item.category, item.key, item.value);
  });

  if (ec) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse world: {}", ec.message());
    return ec;
  }

  return std::move(currentLoadContext.map);
}
}// namespace e00::impl
