#pragma once

#include <Engine/Platform/ResourceLoaderOptions.hpp>
#include <Engine/Platform/Stream.hpp>

namespace e00 {
class ResourceManager;

/**
 * @class ResourceLoader
 * @brief A utility class responsible for loading and managing external resources such as files, images, or configurations.
 *
 * The ResourceLoader class provides an interface for efficiently loading and retrieving external assets
 * required by an application, ensuring proper error handling and resource management.
 *
 * This class is typically used to abstract the complexities of loading various types of external resources
 * and provides centralized management to avoid redundancy in handling resources across the application.
 */
class ResourceLoader {
protected:
  ResourceManager *_engine = nullptr;

public:
  virtual ~ResourceLoader() = default;

  struct Result {
    std::error_code error;
    std::unique_ptr<Resource> resource;

    template<typename T>
    Result(std::unique_ptr<T> &&r) : resource(std::move(r)) {
      static_assert(std::is_base_of_v<Resource, T>, "Class must be of type resource");
    }

    Result(std::error_code ec) : error(ec), resource(nullptr) {}
    Result(std::unique_ptr<Resource> &&r) : resource(std::move(r)) {}

    //explicit operator bool() const { return error.operator bool(); }
    explicit operator Resource *() const { return resource.get(); }

    [[nodiscard]] bool IsType(type_t type) const { return resource && resource->Type() == type; }

    template<typename T>
    [[nodiscard]] bool IsType() const { return IsType(type_id<T>()); }
  };

  struct LoadContext {
    Stream &stream;
    type_t targetTypeId;
    std::span<LoadOption *const> options;

    /**
     * Helper function to find an option of type T in the options list.
     * 
     * @tparam T The option to fin
     * @return the option, or nullptr if not present
     */
    template<typename T>
    const T *ContainsOption() const {
      static_assert(std::is_base_of_v<LoadOption, T>, "Class must be of type LoadOption");

      for (const auto &option: options) {
        if (option->optionTypeid == type_id<T>()) {
          return static_cast<T *>(option);
        }
      }

      return nullptr;
    }
  };

  void SetResourceLoader(ResourceManager *loader) { _engine = loader; }

  virtual explicit operator bool() const { return true; }
  [[nodiscard]] virtual bool SupportsType(type_t type) const = 0;
  [[nodiscard]] virtual bool SupportsOption(type_t optionTypeid) const { return false; }
  virtual bool CanLoad(const LoadContext &context) = 0;
  virtual Result ReadLoad(const LoadContext &context) = 0;
};
}// namespace e00
