#pragma once

#include <Engine.hpp>

#include <string>
#include <system_error>
#include <memory>

namespace e00 {
class ResourceLoader {
protected:
  Engine* _engine = nullptr;

public:
  virtual ~ResourceLoader() = default;

  struct Result {
    std::error_code error;
    std::unique_ptr<Resource> resource;

    Result(std::error_code ec) : error(ec), resource(nullptr) {}
    Result(std::unique_ptr<Resource> &&r) : error(), resource(std::move(r)) {}
    explicit operator bool () const { return error.operator bool(); }
    explicit operator Resource*() const { return resource.get(); }
  };

  void SetResourceLoader(Engine* loader) { _engine = loader; }

  virtual bool SupportsType(type_t type) const = 0;

  virtual bool CanLoad(Stream &stream) = 0;

  virtual ResourceLoader::Result ReadLoad(Stream& stream) = 0;
};
}// namespace e00
