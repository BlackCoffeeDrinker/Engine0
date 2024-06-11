
#include "ResourceManager.hpp"
#include "Resource/ResourceLoader.hpp"


namespace e00 {
ResourceManager::ResourceManager() : _main_logger() {
}


ResourceManager::~ResourceManager() {
}

void ResourceManager::AddLoader(std::unique_ptr<ResourceLoader> &&loaderToAdd) {
  _loaders.push_back(std::move(loaderToAdd));
}

detail::ControlBlock *
ResourceManager::MakeResourceContainer(const std::string &name, type_t type, const source_location &from) {
  for (const auto &a: _loaded_resources_cb) {
    if (a->type() == type && a->name() == name) {
      _main_logger.Info(source_location::current(), "Resource {} of type {} already known", name, type);
      return a.get();
    }
  }

  class R : public detail::ControlBlock {
    ResourceManager *_owner;
    type_t _type;
    std::string _name;
    uint32_t _refs;
    std::unique_ptr<Resource> _resource;

  public:
    R(ResourceManager *e, type_t t, std::string n, source_location /*s*/)
        : _owner(e),
          _type(t),
          _name(std::move(n)),
          _refs(0),
          _resource(nullptr) /*, _init(s) */ {
    }

    ~R() override = default;

    [[nodiscard]] std::string name() const override { return _name; }

    [[nodiscard]] type_t type() const override { return _type; }

    void add_ref() noexcept override { _refs++; }

    void remove_ref() noexcept override {
      if (_refs > 0) {
        _refs--;
      }

      if (_refs == 0) {
        _owner->_main_logger.Info(source_location::current(), "Unloading {}", _name);

        // Find ourselves
        auto i = std::find_if(
            _owner->_loaded_resources_cb.begin(),
            _owner->_loaded_resources_cb.end(),
            [this](const auto &uptr) {
              return uptr.get() == this;
            });

        if (i != _owner->_loaded_resources_cb.end()) {
          _owner->_loaded_resources_cb.erase(i);
        } else {
          _owner->_main_logger.Error(source_location::current(), "Failed to find cache for resource {}",
                                     _name);
        }
      }
    }

    Resource *resource() override {
      if (!_resource) {
        _resource = _owner->LoadRawResource(_name, _type);
      }

      return _resource.get();
    }
  };

  // We didn't find it so make a new container
  auto const &ret = _loaded_resources_cb.emplace_back(std::make_unique<R>(this, type, name, from));

  _main_logger.Info(source_location::current(),
                    "New resource {} of type {} asked at {}:{}",
                    name,
                    type,
                    from.file_name(),
                    from.line());

  return ret.get();
}

std::unique_ptr<Resource> ResourceManager::LoadRawResource(const std::string &name, type_t type) {
  // Open the resource
  if (auto stream = platform::OpenStream(name)) {
    // Loop on all the loaders to find one that can open said resource
    for (const auto &loader: _loaders) {
      // Reset the position of the stream as we could have abandoned it mid read
      if (const auto seekEc = stream->seek(0)) {
        // we failed to seek, this ain't good!
        // log & abandon load
        _main_logger.Error(source_location::current(), "Failed to seek stream while loading resource: {}", seekEc.message());
        return nullptr;
      }

      // Does the loader support this type ?
      if (!loader->SupportsType(type)) {
        continue;
      }

      // Does the loader report it can load this stream ?
      if (!loader->CanLoad(*stream)) {
        // Reset the stream and continue to the next
        continue;
      }

      auto result = loader->ReadLoad(*stream);
      if (result.error) {
        _main_logger.Error(source_location::current(),
                           "Resource loader failed to load resource: {}",
                           result.error.message());
        continue;
      }

      _main_logger.Info(source_location::current(), "Resource {} loaded", name);
      return std::move(result.resource);
    }
  }

  // Something went wrong :(
  _main_logger.Error(source_location::current(),
                     "Unable to load resource {}: data was not found", name);
  return nullptr;
}

void ResourceManager::Tick(const std::chrono::milliseconds &delta) noexcept {
}

}// namespace e00
// e00