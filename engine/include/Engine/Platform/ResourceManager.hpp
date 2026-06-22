#pragma once

#include <chrono>
#include <expected>
#include <list>
#include <memory>
#include <string>
#include <system_error>

#include <Engine/Logging/SourceLocation.hpp>
#include <Engine/Platform/ResourceLoaderOptions.hpp>
#include <Engine/Platform/Stream.hpp>
#include <Engine/Platform/StreamFactory.hpp>
#include <Engine/ResourcePtr.hpp>

namespace e00 {
class ResourceLoader;

/**
 * Manages resources and their lifecycle within the application.
 * Provides methods for loading, accessing, and releasing various types
 * of resources such as files, assets, or other data.
 * 
 * The resource manager is responsible for coordinating the loading,
 * management, and unloading of resources, ensuring efficient resource
 * utilization and preventing memory leaks.
 * 
 * It is also responsible for unloading and lazy loading resources
 * to optimize memory usage
 */
class ResourceManager {
  struct AliasEntry {
    ResourceId id;
    std::string filename;

    // Enables high-speed binary searching via std::lower_bound
    bool operator<(const AliasEntry &other) const { return id < other.id; }
  };

  // Where to open the streams from, defaults to the global stream factory
  StreamFactory &_stream_factory;

  std::vector<AliasEntry> _aliases;
  std::vector<std::unique_ptr<detail::ControlBlock>> _loaded_resources_cb;// << A lightweight control block tracking system
  std::list<std::unique_ptr<ResourceLoader>> _loaders;                    // << all the known loaders

  /**
   * Finds a control block for a given resource id and type
   * 
   * @param id the resource id to search for
   * @param type the type of the resource to search for
   * @return the control block if found, nullptr otherwise
   */
  detail::ControlBlock *FindKnownControlBlockFor(ResourceId id, type_t type) const;

  /**
   * Create a non-lazy control block container
   * 
   * @param id The resource id to create a control block for
   * @param type The type of the resource to create a control block for
   * @param theResource The resource to create a control block for
   * @param from The source location of the call to create the control block
   * @return The control block that was created
   */
  detail::ControlBlock *MakeMemoryContainer(
      ResourceId id,
      type_t type,
      std::unique_ptr<Resource> &&theResource,
      const source_location &from);

  /**
   * Does a quick check to see if the resource can be loaded at a later time
   * 
   * @param id The ID of the resource to check
   * @param type The type of the resource to check
   * @return true if there's a reasonable expectation that the resource can be loaded
   */
  bool CanLoad(ResourceId id, type_t type);

  /**
   * Load a resource from a control block
   * 
   * @param resource_id
   * @param resource_type
   * @param options The options to use when loading the resource
   * @return The loaded resource, or an error code if the resource could not be loaded
   */
  std::expected<std::unique_ptr<Resource>, std::error_code> LoadResource(
      ResourceId resource_id,
      type_t resource_type,
      std::span<LoadOption *const> options);

  /**
   * Delete the control block `cb`.
   * After this call `cb` is no longer valid.
   * 
   * @param cb The control block to erase
   * @return true if the control block was successfully erased, false otherwise
   */
  bool EraseControlBlock(detail::ControlBlock *cb);

  ResourceManager();

  template<typename T, typename... Options>
  ResourcePtrT<T> InternalLazyResource(ResourceId id, source_location from, Options &&...options) {
    if (auto *cb = FindKnownControlBlockFor(id, type_id<T>())) {
      return ResourcePtrT<T>(cb);
    }

    // Check if a stream exists for this resource
    if (!CanLoad(id, type_id<T>())) {
      GetDefaultLogger().Error(source_location::current(), "Unable to load resource {}: data was not found", id);
      return nullptr;
    }

    class StaticOptionControlBlock : public detail::ControlBlock {
      const ResourceManager *const _owner;
      const source_location _from;
      const std::array<LoadOption *, sizeof...(Options)> _opts;

    public:
      StaticOptionControlBlock(ResourceManager *e, ResourceId n, source_location s, Options &&...opts)
          : ControlBlock(n, type_id<T>()),
            _owner(e),
            _from(s),
            _opts{new std::decay_t<Options>(std::forward<Options>(opts))...} {}

      ~StaticOptionControlBlock() override {
        for (auto *p: _opts) delete p;
        _owner->EraseControlBlock(this);
      }

    protected:
      void OnZeroShared() noexcept override {
        GetDefaultLogger().Info(
            source_location::current(),
            "Resource {} of type {} from {}:{} destroyed", id(), type(), _from.file_name(), _from.line());
        delete _resource;
        _resource = nullptr;
      }

    public:
      std::error_code OnLoadLazyResource() override {
        if (auto resource = _owner->LoadResource(
                id(),
                type(),
                std::span<LoadOption *const>(_opts));
            resource) {
          _resource = resource.value().release();
          return {};
        }
        return std::make_error_code(std::errc::no_such_file_or_directory);
      }
    };

    // We know a loader exists for this type and the stream exists, assume it's fine
    auto const &ret = _loaded_resources_cb.emplace_back(
        std::make_unique<StaticOptionControlBlock>(
            this, type_id<T>(), id, from,
            std::forward<Options>(options)...));
    return ResourcePtrT<T>(ret.get());
  }

public:
  static ResourceManager &GlobalResourceManager();
  static ResourceLoader &InvalidLoader();

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
   * Sets an alias for a resource, allowing it to be accessed by resource id
   *
   * @param id The resource identifier
   * @param real_name The new alias name for the resource.
   */
  void SetAlias(ResourceId id, std::string_view real_name);

  /**
   * Searches for an appropriate stream that contains the specified resource.
   * Uses resource identifiers to locate and return the corresponding
   * data stream, facilitating resource access within the application.
   *
   * @param id The name of the resource
   * @param type The type of resource
   * @return A stream object representing the located resource, or null if no
   *         matching stream is found.
   */
  std::unique_ptr<Stream> FindStreamForResource(ResourceId id, type_t type = any());

  /**
   * Convenience method to add a loader to the resource manager.
   * 
   * @tparam T Loader class
   * @tparam Args Constructor arguments type for T
   * @param args Values to pass to the constructor
   * @return Errors, if any
   */
  template<typename T, typename... Args>
  std::error_code AddLoader(Args &&...args) {
    auto loader = std::make_unique<T>(std::forward<Args>(args)...);
    return AddLoader(std::move(loader));
  }

  /**
   * Transfers ownership of a resource to the resource manager.
   * The resulting resource cannot be loaded lazily, nor can it be deleted from memory if memory pressure is high
   * 
   * @tparam T The resource type
   * @param ptr The pointer to the resource to take ownership of
   * @param from source location to help with debugging
   * @return a resource owned by this resource manager
   */
  template<typename T>
  ResourcePtrT<T> TakeOwnership(std::unique_ptr<T> &&ptr, const source_location &from = source_location::current()) {
    return ResourcePtrT<T>(MakeMemoryContainer(0, type_id<T>(), std::move(ptr), from));
  }

  template<typename T>
  ResourcePtrT<T> LazyResource(ResourceId id, const source_location &from = source_location::current()) {
    return InternalLazyResource<T>(id, from);
  }

  template<typename T, typename... Options>
  ResourcePtrT<T> LazyResource(ResourceId id, Options &&...options) {
    static_assert((std::derived_from<std::decay_t<Options>, LoadOption> && ...),
                  "All Options must derive from LoadOption");
    return InternalLazyResource<T>(id, source_location::current(), std::forward<Options>(options)...);
  }

  template<typename T>
  ResourcePtrT<T> LoadResourceDirectly(ResourceId id, const source_location &from = source_location::current()) {
    if (auto *cb = FindKnownControlBlockFor(id, type_id<T>())) {
      // Dont reload if already known
      return ResourcePtrT<T>(cb);
    }

    // Only do something if the resource is loaded
    if (auto resource = LoadResource(id, type_id<T>(), {}); resource) {
      return ResourcePtrT<T>(MakeMemoryContainer(id, type_id<T>(), std::move(resource.value()), from));
    }

    return nullptr;
  }

  template<typename T, typename... Options>
    requires(sizeof...(Options) > 0)
  ResourcePtrT<T> LoadResourceDirectly(ResourceId id, Options &&...options) {
    static_assert((std::derived_from<std::decay_t<Options>, LoadOption> && ...),
                  "All Options must derive from LoadOption");

    // Dont reload if already known
    if (auto *cb = FindKnownControlBlockFor(id, type_id<T>())) {
      return ResourcePtrT<T>(cb);
    }

    // 1. Inline storage for the actual option objects
    auto option_storage = std::tuple<std::decay_t<Options>...>(std::forward<Options>(options)...);

    // 2. Array of LoadOption* pointing into the tuple
    std::array<LoadOption *, sizeof...(Options)> option_ptrs{&std::get<std::decay_t<Options>>(option_storage)...};

    // Only do something if the resource is loaded
    if (auto resource = LoadResource(id, type_id<T>(), option_ptrs); resource) {
      return ResourcePtrT<T>(MakeMemoryContainer(id, type_id<T>(), std::move(resource.value()), source_location::current()));
    }

    return nullptr;
  }

  void Tick(std::chrono::milliseconds delta);
};
}// namespace e00
