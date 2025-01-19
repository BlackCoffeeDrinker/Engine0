#pragma once

namespace e00 {
class ResourceLoader;

class ResourceManager {
  Logger _logger;

  std::list<std::unique_ptr<detail::ControlBlock>> _loaded_resources_cb;// << TODO: Find better container
  std::list<std::unique_ptr<ResourceLoader>> _loaders;                  //< all the known loaders

  detail::ControlBlock *MakeResourceContainer(const std::string &name, type_t type, const source_location &from);

  detail::ControlBlock *MakeMemoryContainer(const std::string &name, type_t type, std::unique_ptr<Resource> &&theResource);

  bool EraseControlBlock(detail::ControlBlock *cb);

  ResourceManager();

protected:
public:
  static ResourceManager &GlobalResourceManager();

  ~ResourceManager() = default;

  /**
   * Adds a loader to the system to manage loading operations.
   *
   * @param loaderToAdd The loader instance to be added. Must be non-null.
   * @return A boolean indicating if the loader was successfully added.
   *         Returns true if the addition was successful, false otherwise.
   */
  std::error_code AddLoader(std::unique_ptr<ResourceLoader> &&loaderToAdd);

  /**
   * 
   * @tparam T 
   * @tparam Args 
   * @param args 
   * @return 
   */
  template<typename T, typename... Args>
  std::error_code AddLoader(Args &&...args) {
    auto loader = std::make_unique<T>(std::forward<Args>(args)...);
    return AddLoader(std::move(loader));
  }

  template<typename T>
  ResourcePtrT<T> LazyResource(const std::string &name, const source_location &from = source_location::current()) {
    return ResourcePtrT<T>(MakeResourceContainer(name, type_id<T>(), from));
  }

  template<typename T, typename... ConstructArgs>
  ResourcePtrT<T> Make(const std::string &name, ConstructArgs &&...args) {
    return ResourcePtrT<T>(MakeMemoryContainer(name, type_id<T>(), std::make_unique<T>(std::forward<ConstructArgs>(args)...)));
  }

  template<typename T>
  ResourcePtrT<T> TakeOwnership(std::unique_ptr<T> &&ptr) {
    return ResourcePtrT<T>(MakeMemoryContainer(ptr->Name(), type_id<T>(), std::move(ptr)));
  }
};


}// namespace e00
