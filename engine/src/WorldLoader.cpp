#include "WorldLoader.hpp"
#include "EngineError.hpp"
#include "IniParser.hpp"

#include <charconv>

namespace {
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
WorldLoader::WorldLoader(ResourceManager *manager) : _engine(manager) {}
WorldLoader::~WorldLoader() = default;

std::error_code WorldLoader::HandleActorData(std::string_view actor_name, std::string_view key, std::string_view value) {
  if (!currentLoadContext.actors.contains(std::string(actor_name))) {
    currentLoadContext.actors[std::string(actor_name)] = {};
  }

  auto &actor = currentLoadContext.actors[std::string(actor_name)];
  if (key == "source") {
    if (!actor.source.empty()) {
      GetDefaultLogger().Error(source_location::current(), "Actor {} already has a source! Duplicate name ?", actor_name);
      return std::make_error_code(std::errc::invalid_argument);
    }
    actor.source = std::string(value);
  } else if (key == "position") {
    // Position is <x>, <y> (spaces might be present or not)
    actor.position.x = std::stoi(std::string(value.substr(0, value.find(','))));
    actor.position.y = std::stoi(std::string(value.substr(value.find(',') + 1)));
  } 

  return {};
}

std::error_code WorldLoader::HandleMapData(std::string_view key, std::string_view value) {
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
  } else if (key == "tileset") {
    // Format is <start tile>:<tileset resource name>
    const auto start_tile = value.substr(0, value.find(':'));
    const auto tileset_name = value.substr(value.find(':') + 1);

    size_t start_tile_number;
    if (const auto size_ec = ToSize(start_tile, start_tile_number)) {
      GetDefaultLogger().Error(source_location::current(), "Failed to parse start tile number {}", start_tile);
      return size_ec;
    }

    currentLoadContext.tilesets.emplace_back(start_tile_number, std::string(tileset_name));
    return {};
  }

  return {};
}

std::error_code WorldLoader::HandleSetData(std::string_view key, std::string_view value, bool layer) {
  MapSetParser ctx;
  if (layer) {
    currentLoadContext.aboveSet.resize(currentLoadContext.width * currentLoadContext.height);
    ctx.callback = [&](int tilevalue) -> std::error_code {
      currentLoadContext.aboveSet[currentLoadContext.setAbovePos] = tilevalue;
      currentLoadContext.setAbovePos++;
      return {};
    };
  } else {
    currentLoadContext.groundSet.resize(currentLoadContext.width * currentLoadContext.height);

    ctx.callback = [&](int tilevalue) -> std::error_code {
      if (tilevalue == 0) {
        GetDefaultLogger().Error(source_location::current(), "Invalid tile value: {}", tilevalue);
        return std::make_error_code(std::errc::invalid_argument);
      }
      currentLoadContext.groundSet[currentLoadContext.setDataPos] = tilevalue;
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
  // We have a map here!
  if (category == "map") return HandleMapData(key, value);
  if (category == "set") return HandleSetData(key, value, false);
  if (category == "setaboveplayer") return HandleSetData(key, value, true);
  if (category.starts_with(kActorPrefix)) return HandleActorData(category.substr(kActorPrefix.size()), key, value);

  return {};
}

WorldLoader::CurrentLoadContext WorldLoader::Load(Stream &stream) {
  currentLoadContext = {};

  const auto ec = IniParser::Parse(stream, [&](const IniParser::Item &item) -> std::error_code {
    return this->HandleWorldData(item.category, item.key, item.value);
  });

  if (ec) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse world: {}", ec.message());
    return {};
  }

  return currentLoadContext;
}
}// namespace e00::impl
