#include "PrivateInclude.hpp"

#include "Loaders/BitmapLoader.hpp"
#include "Loaders/WorldLoader.hpp"

namespace {
std::unique_ptr<e00::ResourceManager> _globalResourceManager;

std::unique_ptr<e00::ResourceLoader> CreateLoaderForType(const e00::type_t resourceType) {
  if (resourceType == e00::type_id<e00::Map>()) {
    return std::make_unique<e00::impl::WorldLoader>();
  } else if (resourceType == e00::type_id<e00::Bitmap>()) {
    return std::make_unique<e00::impl::BitmapLoader>();
  }

  return nullptr;
}

class ControlBlockDetails : public e00::detail::ControlBlock {
  static e00::Logger _logger;

  e00::ResourceManager *_owner;
  e00::type_t _type;
  std::string _name;
  uint32_t _refs{0};
  std::unique_ptr<e00::Resource> _resource;

protected:
  [[nodiscard]] auto Owner() const noexcept { return _owner; }

  virtual void ZeroRefs() noexcept = 0;

  virtual void LoadResource() {
    if (const auto loader = CreateLoaderForType(_type)) {
      // Set up the loader
      loader->SetResourceLoader(_owner);

      // Load it (well, try)
      if (const auto stream = e00::StreamFactory::GlobalStreamFactory().OpenStream(_name)) {
        auto result = loader->ReadLoad(*stream);
        if (result.error) {
          _logger.Error(e00::source_location::current(),
                        "Resource loader failed to load resource: {}",
                        result.error.message());
          return;
        }

        _resource = std::move(result.resource);
        _logger.Info(e00::source_location::current(), "Resource {} loaded", _name);
      } else {
        // Something went wrong :(
        _logger.Error(e00::source_location::current(), "Unable to load resource {}: data was not found", _name);
      }

      return;
    }

    _logger.Error(e00::source_location::current(), "Unable to find resource loader");
  }

public:
  ControlBlockDetails(e00::ResourceManager *e, e00::type_t t, std::string n, e00::source_location /*s*/)
      : _owner(e),
        _type(t),
        _name(std::move(n)),
        _resource(nullptr) /*, _init(s) */ {
  }

  ControlBlockDetails(e00::ResourceManager *e, e00::type_t t, std::string n, std::unique_ptr<e00::Resource> &&r)
      : _owner(e),
        _type(t),
        _name(std::move(n)),
        _resource(std::move(r)) {}

  ~ControlBlockDetails() override = default;

  [[nodiscard]] std::string_view name() const override { return _name; }
  [[nodiscard]] e00::type_t type() const override { return _type; }
  void add_ref() noexcept override { _refs++; }
  void remove_ref() noexcept override {
    if (_refs > 0) {
      _refs--;
    }

    if (_refs == 0) {
      _logger.Info(e00::source_location::current(), "Unloading {}", name());
      ZeroRefs();
    }
  }

  e00::Resource *resource() override {
    if (!_resource) {
      LoadResource();
    }

    return _resource.get();
  }
};

e00::Logger ControlBlockDetails::_logger = e00::Logger();

}// namespace

namespace e00 {
ResourceManager &ResourceManager::GlobalResourceManager() {
  if (!_globalResourceManager) {
    _globalResourceManager = std::unique_ptr<ResourceManager>(new ResourceManager);
  }

  return *_globalResourceManager;
}

std::error_code ResourceManager::AddLoader(std::unique_ptr<ResourceLoader> &&loaderToAdd) {
  loaderToAdd->SetResourceLoader(this);
  _loaders.push_back(std::move(loaderToAdd));
  return {};
}

ResourceManager::ResourceManager() = default;

detail::ControlBlock *ResourceManager::MakeMemoryContainer(const std::string &name, type_t type, std::unique_ptr<Resource> &&theResource) {
  for (const auto &a: _loaded_resources_cb) {
    if (a->type() == type && a->name() == name) {
      _logger.Info(source_location::current(), "Resource {} of type {} already known", name, type);
      return a.get();
    }
  }

  class R : public ControlBlockDetails {
  public:
    R(ResourceManager *e, type_t t, std::string n, std::unique_ptr<Resource> &&r)
        : ControlBlockDetails(e, t, std::move(n), std::move(r)) {}

    ~R() override = default;

  protected:
    void ZeroRefs() noexcept override {
      if (!Owner()->EraseControlBlock(this)) {
        Owner()->_logger.Error(source_location::current(), "Failed to find cache for resource {}", name());
      }
    }
  };

  auto const &ret = _loaded_resources_cb.emplace_back(std::make_unique<R>(this, type, name, std::move(theResource)));

  _logger.Info(source_location::current(), "Memory create resource {} of type {}", name, type);
  return ret.get();
}

detail::ControlBlock *ResourceManager::MakeResourceContainer(const std::string &name, type_t type, const source_location &from) {
  for (const auto &a: _loaded_resources_cb) {
    if (a->type() == type && a->name() == name) {
      _logger.Info(source_location::current(), "Resource {} of type {} already known", name, type);
      return a.get();
    }
  }

  class R : public ControlBlockDetails {
  public:
    R(ResourceManager *e, type_t t, std::string n, source_location s)
        : ControlBlockDetails(e, t, std::move(n), s) {
    }

    ~R() override = default;

  protected:
    void ZeroRefs() noexcept override {
      if (!Owner()->EraseControlBlock(this)) {
        Owner()->_logger.Error(source_location::current(), "Failed to find cache for resource {}", name());
      }
    }
  };

  // Did not find the resource; so make a new container
  auto const &ret = _loaded_resources_cb.emplace_back(std::make_unique<R>(this, type, name, from));

  _logger.Info(source_location::current(),
               "New resource {} of type {} asked at {}:{}",
               name,
               type,
               from.file_name(),
               from.line());

  return ret.get();
}

bool ResourceManager::EraseControlBlock(detail::ControlBlock *cb) {
  // Find the control block
  const auto i = std::find_if(
      _loaded_resources_cb.begin(),
      _loaded_resources_cb.end(),
      [&cb](const auto &uptr) {
        return uptr.get() == cb;
      });

  if (i != _loaded_resources_cb.end()) {
    _loaded_resources_cb.erase(i);
    return true;
  }

  return false;
}


}// namespace e00
