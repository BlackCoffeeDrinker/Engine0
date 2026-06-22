#include "WorldLoader.hpp"
#include "EngineError.hpp"
#include "IniParser.hpp"

#include <charconv>

namespace {
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
      if (item.key == "source") {
        if (map->Tileset()) {
          GetDefaultLogger().Error(source_location::current(), "Tileset already set");
          return std::make_error_code(std::errc::invalid_argument);
        }

        map->SetTileset(_engine->LoadResourceDirectly<Bitmap>(HashName(item.value)));
        if (!map->Tileset()) {
          GetDefaultLogger().Error(source_location::current(), "Failed to load bitmap {}", item.value);
          return std::make_error_code(std::errc::invalid_argument);
        }
      }
    }

    if (item.category == "tiles") {
      if (item.key == "tilewidth") {
        size_t tileWidth;
        if (const auto size_ec = ToSize(item.value, tileWidth)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse tilewidth {}", item.value);
          return size_ec;
        }
        auto tileSize = map->TileSize();
        tileSize.x = tileWidth;
        map->SetTileSize(tileSize);
      }
      if (item.key == "tileheight") {
        size_t tileHeight;
        if (const auto size_ec = ToSize(item.value, tileHeight)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse tileheight {}", item.value);
          return size_ec;
        }
        auto tileSize = map->TileSize();
        tileSize.y = tileHeight;
        map->SetTileSize(tileSize);
      }
    }

    if (item.category.starts_with("tile:")) {
      // Per tile options
      GetDefaultLogger().Info(source_location::current(), "Tile: {} -> {} = {}", item.category, item.key, item.value);
    }

    return {};
  });
}

std::error_code WorldLoader::ParseSet(Stream &stream, const std::unique_ptr<Map> &map) {
  stream.SeekTo(0);

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
      map->Set(idx, current);

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
    map->Set(idx, current);
  }

  return {};
}


bool WorldLoader::CanLoad(const LoadContext& context) {
  return true;
}

ResourceLoader::Result WorldLoader::ReadLoad(const LoadContext& context) {
  size_t width = 0;
  size_t height = 0;
  std::unique_ptr<Map> map;

  const auto ec = IniParser::Parse(context.stream, [&](const IniParser::Item &item) -> std::error_code {
    if (item.category == "map" && !map) {
      if (item.key == "width") {
        if (const auto size_ec = ToSize(item.value, width)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse width {}", item.value);
          return size_ec;
        }
        if (width != 0 && height != 0) {
          map = std::make_unique<Map>(width, height);
        }
        return {};
      }

      if (item.key == "height") {
        if (const auto size_ec = ToSize(item.value, height)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse height {}", item.value);
          return size_ec;
        }

        if (width != 0 && height != 0) {
          map = std::make_unique<Map>(width, height);
        }
        return {};
      }
    }

    if (!map) {
      GetDefaultLogger().Error(source_location::current(), "World is not valid, missing key 'width' or 'height' in 'map' category before any other options");
      return std::make_error_code(std::errc::invalid_argument);
    }

    if (item.category == "map") {
      if (item.key == "tileset") {
        // Load Set metadata
        if (const auto &tileset = _engine->FindStreamForResource(HashName(item.value))) {
          return ParseTileset(*tileset, map);
        }
        GetDefaultLogger().Error(source_location::current(), "Failed to FindStreamForResource tileset {}", item.value);
        return std::make_error_code(std::errc::invalid_argument);
      }
      if (item.key == "set") {
        if (const auto &set = _engine->FindStreamForResource(HashName(item.value))) {
          return ParseSet(*set, map);
        }
        GetDefaultLogger().Error(source_location::current(), "Failed to FindStreamForResource set {}", item.value);
        return std::make_error_code(std::errc::invalid_argument);
      }
      if (item.key == "music") {
        // TODO
      }
    }

    return {};
  });

  if (ec) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse world: {}", ec.message());
    return ec;
  }

  return map;
}
}// namespace e00::impl
