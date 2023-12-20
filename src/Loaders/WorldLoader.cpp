#include "WorldLoader.hpp"
#include "EngineError.hpp"
#include "nlohmann/json.hpp"

namespace e00::impl {
class WorldLoader::Impl {
  mutable Logger _logger;
  nlohmann::json _world_json;

public:
  Impl() : _logger(CreateSink("WorldLoader")){};
  ~Impl() = default;

  template<typename InputType>
  void parse(InputType &&i) { _world_json = nlohmann::json::parse(std::forward<InputType>(i), nullptr, false, true); }

  template<typename KeyType>
  bool contains(const KeyType &key) const {
    if (!_world_json.contains(key)) {
      _logger.Log(source_location::current(), L_ERROR, "World is not valid, missing key {}", std::string_view{ key });
      return false;
    }
    return true;
  }

  template<typename... KeyType>
  bool contains_all(const KeyType &...keys) const {
    return (contains(keys) && ...);
  }

  template<typename KeyType, typename T>
  bool get(const KeyType &key, T &output) const {
    if (const auto &a = _world_json.find(key);
        a != _world_json.end()) {
      output = (*a).template get<T>();
      return true;
    }

    _logger.Log(source_location::current(), L_ERROR, "World is not valid, missing key {}", std::string_view{ key });
    return false;
  }

  [[nodiscard]] auto &world_json() const { return _world_json; }
};

WorldLoader::WorldLoader()
  : ResourceLoader(),
    _logger(CreateSink("WorldLoader")),
    _impl(std::make_unique<WorldLoader::Impl>()) {
}

WorldLoader::~WorldLoader() {
  _impl.reset();
}

bool WorldLoader::CanLoad(Stream &stream) {
  return true;
}

ResourceLoader::Result WorldLoader::ReadLoad(Stream &stream) {
  // TODO: Don't load everything into memory twice
  std::string json_str;
  json_str.resize(stream.stream_size());
  if (auto ec = stream.read(stream.stream_size(), json_str.data())) {
    return ec;
  }

  // Parse the world file & make sure it's valid
  _impl->parse(json_str);

  int width = 0;
  int height = 0;

  if (!_impl->get("width", width) || !_impl->get("height", height)) {
    _logger.Info(source_location::current(), "Width, Height is not present");
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Create the map!
  auto theMap = std::make_unique<Map>(width, height);

  // Read the map
  const auto &layers = _impl->world_json().find("layers");
  if (layers == _impl->world_json().end() || !layers->is_array()) {
    _logger.Info(source_location::current(), "Layers isn't array");
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Read all the layers
  // TODO: this only reads one for now
  for (const auto &layer : *layers) {
    auto layer_width = layer.find("width")->get<int>();
    auto layer_height = layer.find("height")->get<int>();
    auto layer_x = layer.find("x")->get<int>();
    auto layer_y = layer.find("y")->get<int>();
    auto layer_data = layer.find("data");

    if (layer_width != width || layer_height != height) {
      // TODO
      _logger.Info(source_location::current(), "Width, Height is not present in layer");
      return std::make_error_code(std::errc::invalid_argument);
    }

    if (layer_data == layer.end() || !layer_data->is_array()) {
      // TODO
      _logger.Info(source_location::current(), "Layer data is not an array");
      return std::make_error_code(std::errc::invalid_argument);
    }

    auto current = layer_data->begin();
    auto last = layer_data->end();

    // Make sure it's the right size
    if (std::distance(current, last) != theMap->Size().Area()) {
      // TODO
      _logger.Info(source_location::current(), "Layer data is the wrong size");
      return std::make_error_code(std::errc::invalid_argument);
    }

    // Copy the data
    uint32_t i = 0;
    while (current != last) {
      theMap->Set(Map::Position(i % layer_width, i / layer_width), current->get<uint16_t>());
      i++;
      current++;
    }

    break;
  }

  // Ref the tileset
  const auto &tilesets = _impl->world_json().find("tilesets");
  if (tilesets == _impl->world_json().end() || !tilesets->is_array()) {
    _logger.Error(source_location::current(), "No tileset in file");
    return std::make_error_code(std::errc::invalid_argument);
  }

  for (const auto &tileset_json : *tilesets) {
    const auto firstgid = tileset_json.find("firstgid");
    const auto image = tileset_json.find("image");
    const auto imageheight = tileset_json.find("imageheight");
    const auto imagewidth = tileset_json.find("imagewidth");
    const auto tilecount = tileset_json.find("tilecount");
    const auto tileheight = tileset_json.find("tileheight");
    const auto tilewidth = tileset_json.find("tilewidth");
    const auto spacing = tileset_json.find("spacing");
    const auto margin = tileset_json.find("margin");

    Tileset tileset(tilecount->get<uint16_t>());
    tileset.SetBitmap(_engine->LazyResource<Bitmap>(image->get<std::string>()));
    tileset.SetSpacing(spacing->get<uint16_t>());
    tileset.SetMargin(margin->get<uint16_t>());
    tileset.SetTilesize({ tilewidth->get<uint16_t>(), tileheight->get<uint16_t>() });

    theMap->SetTileset(std::move(tileset));
    break;
  }

  // Do some checks
  if (theMap->HighestTitleId() > theMap->Tileset().NumberOfTiles()) {
    _logger.Error(source_location::current(), "Inconsistent number of tiles, maps has ID {} but tileset knows about {}", theMap->HighestTitleId(), theMap->Tileset().NumberOfTiles());

    return std::make_error_code(std::errc::invalid_argument);
  }

  _logger.Info(source_location::current(), "Map loaded successfully");
  return Result(std::move(theMap));
}
}// namespace e00::impl
