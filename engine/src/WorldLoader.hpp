#pragma once

#include <Engine.hpp>

namespace e00::impl {

class WorldLoader {
  ResourceManager *_engine = nullptr;

public:
  struct TilesetDef {
    size_t startTileId = 0;
    std::string tilesetResourceName;
  };
  
  struct ActorDef {
    std::string source;
    WorldPosition position;
    std::map<std::string, std::string> default_properties;
  };

  struct CurrentLoadContext {
    size_t width = 0;
    size_t height = 0;
    size_t setDataPos = 0;
    size_t setAbovePos = 0;
    std::vector<TilesetDef> tilesets;
    
    std::vector<TileIdType> groundSet;
    std::vector<TileIdType> aboveSet;
    
    std::map<std::string, ActorDef> actors;
    std::map<std::string, WorldPosition> entries;
  };

private:
  std::error_code HandleActorData(std::string_view actor_name, std::string_view key, std::string_view value);
  std::error_code HandleEntryData(std::string_view entry_name, std::string_view key, std::string_view value);
  std::error_code HandleWorldData(std::string_view category, std::string_view key,
                                  std::string_view value);

  std::error_code HandleMapData(std::string_view key, std::string_view value);
  std::error_code HandleSetData(std::string_view key, std::string_view value, bool layer);

  CurrentLoadContext currentLoadContext;

public:
  explicit WorldLoader(ResourceManager *manager);
  ~WorldLoader();

  CurrentLoadContext Load(Stream& stream);
};
}// namespace e00::impl
