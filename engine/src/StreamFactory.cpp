#include "PrivateInclude.hpp"
#include "Platform.hpp"

namespace {
std::unique_ptr<e00::StreamFactory> _streamFactory;

class RootStreamFactory : public e00::StreamFactory {
  std::string _resource_directory;

public:
  RootStreamFactory() : _resource_directory("res/") {};

  ~RootStreamFactory() override = default;

  std::unique_ptr<e00::Stream> OpenStream(const std::string &name) override {
    return platform::OpenStream(_resource_directory + name);
  }

  std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string &name) override {
    return platform::OpenStreamForWrite(_resource_directory + name);
  }

  void SetResourceDirectory(const std::string &path) override {
    _resource_directory = path;
  }
};

}// namespace

namespace e00 {
StreamFactory &StreamFactory::GlobalStreamFactory() {
  if (!_streamFactory) {
    _streamFactory = std::unique_ptr<StreamFactory>(new RootStreamFactory);
  }
  return *_streamFactory;
}

}// namespace e00
