
#include "ActorLoader.hpp"
#include "IniParser.hpp"

namespace {
constexpr std::string_view kStateCategory = "state:";
}

namespace e00::impl {

ActorLoader::ActorLoader(ResourceManager *manager) : _engine(manager) {
}

std::error_code ActorLoader::HandleActorData(std::string_view key, std::string_view value) {
  if (key == "type") currentData.type = value;
  if (key == "interactable") currentData.interactable = value;
  if (key == "size") {
    // Size is <width>, <height> (spaces might be present or not)
    currentData.size.x = std::stoi(std::string(value.substr(0, value.find(','))));
    currentData.size.y = std::stoi(std::string(value.substr(value.find(',') + 1)));
  }
  return {};
}

std::error_code ActorLoader::HandleInstanceData(std::string_view key, std::string_view value) {
  return {};
}

std::error_code ActorLoader::HandleStateData(std::string_view stateName, std::string_view key, std::string_view value) {
  GetDefaultLogger().Info(source_location::current(), "State: {}, Key: {}, Value: {}", stateName, key, value);
  return {};
}

std::error_code ActorLoader::HandleData(std::string_view category, std::string_view key, std::string_view value) {
  if (category == "actor") return HandleActorData(key, value);
  if (category == "instance") return HandleInstanceData(key, value);
  if (category.starts_with(kStateCategory)) return HandleStateData(category.substr(kStateCategory.size()), key, value);
}

std::unique_ptr<Actor> ActorLoader::LoadActor(Stream &stream) {
  currentData = {};

  const auto ec = IniParser::Parse(stream, [&](const IniParser::Item &item) -> std::error_code {
    return this->HandleData(item.category, item.key, item.value);
  });

  if (ec) {
    GetDefaultLogger().Error(source_location::current(), "Failed to parse world: {}", ec.message());
    return {};
  }

  return nullptr;
}
}// namespace e00::impl
