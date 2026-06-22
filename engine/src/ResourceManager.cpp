#include <utility>

#include "PrivateInclude.hpp"

namespace {
std::unique_ptr<e00::ResourceManager> _globalResourceManager;
}// namespace

namespace e00 {
ResourceManager &ResourceManager::GlobalResourceManager() {
  if (!_globalResourceManager) {
    _globalResourceManager = std::unique_ptr<ResourceManager>(new ResourceManager);
  }

  return *_globalResourceManager;
}

ResourceLoader &ResourceManager::InvalidLoader() {
  class InvalidLoader : public ResourceLoader {
  public:
    [[nodiscard]] bool SupportsType(type_t type) const override { return false; }
    bool CanLoad(const LoadContext &context) override { return false; }
    Result ReadLoad(const LoadContext &context) override { return {std::make_error_code(std::errc::invalid_argument)}; }
    explicit operator bool() const override { return false; }
  };
  static InvalidLoader invalidLoader;
  return invalidLoader;
}

std::error_code ResourceManager::AddLoader(std::unique_ptr<ResourceLoader> &&loaderToAdd) {
  loaderToAdd->SetResourceLoader(this);
  _loaders.push_back(std::move(loaderToAdd));
  return {};
}

void ResourceManager::SetAlias(ResourceId id, std::string_view real_name) {
  _aliases.emplace_back(id, std::string(real_name));
}

ResourceManager::ResourceManager()
    : _stream_factory(StreamFactory::GlobalStreamFactory()) {
}

std::unique_ptr<Stream> ResourceManager::FindStreamForResource(ResourceId id, type_t type) {
  const auto aliasIt = std::ranges::find_if(_aliases, [&](const auto &alias) {
    return alias.id == id;
  });

  if (aliasIt != _aliases.end()) {
    GetDefaultLogger().Info(source_location::current(), "Alias found for {} -> {}", id, aliasIt->filename);
    return _stream_factory.OpenStream(aliasIt->filename);
  }

  GetDefaultLogger().Error(source_location::current(), "No alias found for {}", id);
  return nullptr;
}

detail::ControlBlock *ResourceManager::FindKnownControlBlockFor(ResourceId id, type_t type) const {
  // Do we know about this resource already?
  for (const auto &a: _loaded_resources_cb) {
    if (a->type() == type && a->id() == id) {
      GetDefaultLogger().Info(source_location::current(), "Resource {} of type {} already known", id, type);
      return a.get();
    }
  }

  return nullptr;
}

detail::ControlBlock *ResourceManager::MakeMemoryContainer(ResourceId id, type_t type, std::unique_ptr<Resource> &&theResource, const source_location &from) {
  class R : public detail::ControlBlock {
    ResourceManager *const _owner;

  public:
    R(ResourceManager *e, type_t t, ResourceId n, std::unique_ptr<Resource> &&r)
        : ControlBlock(n, t),
          _owner(e) {
      _resource = r.release();
    }

    ~R() override { _owner->EraseControlBlock(this); }

    std::error_code OnLoadLazyResource() override { return std::make_error_code(std::errc::not_supported); }

  protected:
    void OnZeroShared() noexcept override {
      delete _resource;
      _resource = nullptr;
    }
  };

  auto const &ret = _loaded_resources_cb.emplace_back(
      std::make_unique<R>(
          this,
          type,
          id,
          std::move(theResource)));

  GetDefaultLogger().Info(source_location::current(), "Memory create resource {} of type {}", id, type);
  return ret.get();
}

bool ResourceManager::CanLoad(ResourceId id, type_t type) {
  // Make sure at least one loader can load this type, don't actually do a CanLoad on it as it may be expensive
  if (!std::ranges::any_of(_loaders, [&](const std::unique_ptr<ResourceLoader> &loader) {
        return loader->SupportsType(type);
      })) {
    // No loader found for this type
    GetDefaultLogger().Error(source_location::current(), "Unable to load resource {}: no loader found", id);
    return false;
  }

  // Check if a stream exists for this resource
  if (const auto stream = FindStreamForResource(id, type)) {
    return true;
  }

  GetDefaultLogger().Error(source_location::current(), "Unable to load resource {}: data was not found", id);
  return false;
}

std::expected<std::unique_ptr<Resource>, std::error_code> ResourceManager::LoadResource(ResourceId resource_id, type_t resource_type, std::span<LoadOption *const> options) {
  // Find the stream
  if (const auto stream = FindStreamForResource(resource_id, resource_type)) {
    GetDefaultLogger().Info(source_location::current(), "Loading resource {} of type {}", resource_id, resource_type);

    const ResourceLoader::LoadContext ctx{
        .stream = *stream,
        .targetTypeId = resource_type,
        .options = options};

    // Find a loader
    for (const auto &loader: _loaders) {
      if (!loader->SupportsType(resource_type) || !loader->CanLoad(ctx)) {
        continue;
      }

      if (auto attemptedLoad = loader->ReadLoad(ctx);
          attemptedLoad.resource != nullptr) {
        return std::move(attemptedLoad.resource);
      }
    }

    // No loader was able to load this resource
    GetDefaultLogger().Error(source_location::current(), "No loader was able to load resource {}", resource_id);
    return std::unexpected(std::make_error_code(std::errc::not_supported));
  }

  GetDefaultLogger().Error(source_location::current(), "Unable to load resource {}", resource_id);
  return std::unexpected(std::make_error_code(std::errc::no_such_file_or_directory));
}

bool ResourceManager::EraseControlBlock(detail::ControlBlock *cb) {
  GetDefaultLogger().Info(source_location::current(), "Erasing control block {}", cb->id());

  // Find the control block
  const auto i = std::ranges::find_if(_loaded_resources_cb,
                                      [&cb](const auto &uptr) {
                                        return uptr.get() == cb;
                                      });

  if (i != _loaded_resources_cb.end()) {
    _loaded_resources_cb.erase(i);
    return true;
  }

  return false;
}

void ResourceManager::Tick(const std::chrono::milliseconds delta) {
  // Go through all the resources that need ticks
  std::ignore = delta;
}


}// namespace e00
