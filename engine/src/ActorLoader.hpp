
#pragma once
#include <Engine.hpp>

namespace e00::impl {
class ActorLoader {
  ResourceManager *_engine = nullptr;
public:
  struct ActorData {
    std::string type;
    std::string interactable;
    BitmapSize size;
  };
  
private:
  
  ActorData currentData{};
  
  std::error_code HandleActorData(std::string_view key, std::string_view value);
  std::error_code HandleInstanceData(std::string_view key, std::string_view value);
  std::error_code HandleStateData(std::string_view stateName, std::string_view key, std::string_view value);
  std::error_code HandleData(std::string_view category, std::string_view key,
                                std::string_view value);
public:
  explicit ActorLoader(ResourceManager *manager);
  ~ActorLoader() = default;

  std::unique_ptr<Actor> LoadActor(Stream &stream);
};
}// namespace e00::impl
