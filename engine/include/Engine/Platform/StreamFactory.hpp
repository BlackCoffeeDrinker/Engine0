#pragma once

#include "Stream.hpp"
#include <string>

namespace e00 {
class StreamFactory {
protected:
  StreamFactory() = default;

public:
  virtual ~StreamFactory() = default;

  static StreamFactory &GlobalStreamFactory();

  virtual std::unique_ptr<Stream> OpenStream(const std::string &name) = 0;

  virtual std::unique_ptr<WritableStream> OpenStreamForWrite(const std::string &name) = 0;

  virtual void SetResourceDirectory(const std::string &path) = 0;
};
}// namespace e00
