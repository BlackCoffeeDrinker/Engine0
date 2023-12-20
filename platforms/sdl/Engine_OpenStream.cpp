
#include <Engine.hpp>
#include "StdFile.hpp"

namespace e00 {
std::unique_ptr<Stream> Engine::OpenStream(const std::string &name) {
  return StdFile::CreateFromFilename("resources/" + name);
}
}
