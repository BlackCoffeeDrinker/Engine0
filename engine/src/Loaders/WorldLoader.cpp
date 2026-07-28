#include "WorldLoader.hpp"
#include "EngineError.hpp"
#include "IniParser.hpp"

#include <charconv>

namespace {
constexpr std::string_view kTileIdPrefix = "id:";
constexpr std::string_view kActorPrefix = "object:";

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

struct MapSetParser {
  std::function<std::error_code(int num)> callback;
  int current = -1;
};

std::error_code Parse(MapSetParser &context, char c) {
  if (c >= '0' && c <= '9') {
    if (context.current == -1) {
      context.current = 0;
    }

    context.current = context.current * 10 + (c - '0');
  } else if (c == ',') {
    if (const auto ec = context.callback(context.current)) {
      return ec;
    }
    context.current = -1;
  } else if (context.current != -1) {
    // Ignore invalid characters
  }

  return {};
}

std::error_code ParseEnd(MapSetParser &context) {
  if (context.current != -1) {
    if (const auto ec = context.callback(context.current)) {
      return ec;
    }
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

std::error_code WorldLoader::ParseTileset(Stream &stream, size_t start_tile_number) {
  const auto &map = currentLoadContext.map;
  map->SetTilesetStartingTileId(start_tile_number);

  return IniParser::Parse(stream, [&](const IniParser::Item &item) -> std::error_code {
    if (item.category == "tileset") {
      if (item.key == "source") {
        currentLoadContext.map->SetTileset(_engine->LazyResource<Bitmap>(HashName(item.value)));
      }

      if (item.key == "tilewidth") {
        size_t tileWidth;
        if (const auto size_ec = ToSize(item.value, tileWidth)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse tilewidth {}", item.value);
          return size_ec;
        }
        auto tileSize = currentLoadContext.map->TileSize();
        tileSize.x = tileWidth;
        currentLoadContext.map->SetTileSize(tileSize);
      }

      if (item.key == "tileheight") {
        size_t tileHeight;
        if (const auto size_ec = ToSize(item.value, tileHeight)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse tileheight {}", item.value);
          return size_ec;
        }
        auto tileSize = currentLoadContext.map->TileSize();
        tileSize.y = tileHeight;
        currentLoadContext.map->SetTileSize(tileSize);
      }

      if (item.key == "spacing") {
        size_t spacing;
        if (const auto size_ec = ToSize(item.value, spacing)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse spacing {}", item.value);
          return size_ec;
        }
        currentLoadContext.map->SetTilesetSpacing(spacing);
      }

      if (item.key == "margin") {
        size_t margin;
        if (const auto size_ec = ToSize(item.value, margin)) {
          GetDefaultLogger().Error(source_location::current(), "Failed to parse margin {}", item.value);
          return size_ec;
        }
        currentLoadContext.map->SetTilesetMargin(margin);
      }
    } else if (item.category.starts_with(kTileIdPrefix)) {
      const auto tile_number_str = item.category.substr(kTileIdPrefix.size());
      size_t tile_number;
      if (const auto size_ec = ToSize(tile_number_str, tile_number)) {
        GetDefaultLogger().Error(source_location::current(), "Failed to parse tile number {}", tile_number_str);
        return size_ec;
      }

      GetDefaultLogger().Info(source_location::current(), "Parsing tile type config for tile number {}: {} = {}", tile_number, item.key, item.value);
    }

    return {};
  });
}

std::error_code WorldLoader::HandleActorData(std::string_view actor_name, std::string_view key, std::string_view value) {
  GetDefaultLogger().Info(source_location::current(), "Parsing actor data for actor {}: {} = {}", actor_name, key, value);
  
  return {};
}

std::error_code WorldLoader::HandleMapData(std::string_view key, std::string_view value) {
  if (key == "tileset") {
    // Format is <start tile>:<tileset resource name>
    const auto start_tile = value.substr(0, value.find(':'));
    const auto tileset_name = value.substr(value.find(':') + 1);

    size_t start_tile_number;
    if (const auto size_ec = ToSize(start_tile, start_tile_number)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse start tile number {}", start_tile);
      return size_ec;
    }

    if (const auto &set = _engine->FindStreamForResource(HashName(tileset_name))) {
      return ParseTileset(*set, start_tile_number);
    }

    return {};
  }

  return {};
}

std::error_code WorldLoader::HandleSetData(std::string_view key, std::string_view value, bool layer) {
  MapSetParser ctx;
  if (layer) {
    ctx.callback = [&](int tilevalue) -> std::error_code {
      const Position position(currentLoadContext.setAbovePos % currentLoadContext.map->Size().x,
                              currentLoadContext.setAbovePos / currentLoadContext.map->Size().x);
      currentLoadContext.map->SetAbove(position, tilevalue);
      currentLoadContext.setAbovePos++;
      return {};
    };
  } else {
    ctx.callback = [&](int tilevalue) -> std::error_code {
      if (tilevalue == 0) {
        GetDefaultLogger().Error(source_location::current(), "Invalid tile value: {}", tilevalue);
        return std::make_error_code(std::errc::invalid_argument);
      }
      const Position position(currentLoadContext.setDataPos % currentLoadContext.map->Size().x,
                              currentLoadContext.setDataPos / currentLoadContext.map->Size().x);
      currentLoadContext.map->SetGround(position, tilevalue);
      currentLoadContext.setDataPos++;
      return {};
    };
  }

  if (key == "source") {
    if (const auto &set = _engine->FindStreamForResource(HashName(value))) {
      if (const auto ec = set->SeekTo(0)) { return ec; }
      while (!set->AtEnd()) {
        char c;
        if (const auto ec = set->Read(c)) { return ec; }
        if (const auto ec = Parse(ctx, c)) { return ec; }
      }
      return ParseEnd(ctx);
    }
    GetDefaultLogger().Error(source_location::current(), "Failed to FindStreamForResource set {}", value);
    return std::make_error_code(std::errc::invalid_argument);
  }

  if (key == "data") {
    for (const auto &c: value) {
      if (const auto ec = Parse(ctx, c)) {
        return ec;
      }
    }

    return ParseEnd(ctx);
  }

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
  if (category == "set") return HandleSetData(key, value, false);
  if (category == "setaboveplayer") return HandleSetData(key, value, true);
  if (category.starts_with(kActorPrefix)) return HandleActorData(category.substr(kActorPrefix.size()), key, value);

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
