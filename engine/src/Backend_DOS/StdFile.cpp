
#include "StdFile.hpp"

namespace platform {
std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name) {
  return StdFile::CreateFromFilename(name);
}

std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name) {
  return StdFile::CreateFromFilename(name, true);
}

}
