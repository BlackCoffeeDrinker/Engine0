#include "PrivateInclude.hpp"

namespace {
std::unique_ptr<e00::StreamFactory> _streamFactory;
}

namespace e00 {
StreamFactory::StreamFactory() = default;

StreamFactory::~StreamFactory() = default;

StreamFactory &StreamFactory::GlobalStreamFactory() {
  if (!_streamFactory) {
    _streamFactory = std::unique_ptr<StreamFactory>(new StreamFactory);
  }
  return *_streamFactory;
}

std::unique_ptr<Stream> StreamFactory::OpenStream(const std::string &name) {
  return platform::OpenStream(name);
}


}// namespace e00
