
#pragma once

#include "PrivateInclude.hpp"

namespace e00 {
class ResourceLoader;

class ResourceManager {
  Logger _main_logger;

  std::list<std::unique_ptr<detail::ControlBlock>> _loaded_resources_cb;// << TODO: Find better container
  std::list<std::unique_ptr<ResourceLoader>> _loaders; //< all the known loaders

  std::unique_ptr<Resource> LoadRawResource(const std::string &name, type_t type);

public:
  ResourceManager();
  ~ResourceManager();

  void AddLoader(std::unique_ptr<ResourceLoader>&& loaderToAdd);

  detail::ControlBlock *MakeResourceContainer(const std::string &name,
                                              type_t type,
                                              const source_location &from);

  void Tick(const std::chrono::milliseconds &delta) noexcept;
};

}// namespace e00
