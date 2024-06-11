#include "StdFile.hpp"

#include <memory>

std::unique_ptr<StdFile> StdFile::CreateFromFilename(const std::string &fileName) {
  if (FILE *fp = std::fopen(fileName.c_str(), "rb")) {
    if (std::fseek(fp, 0, SEEK_END) == 0) {
      const auto size = std::ftell(fp);
      std::fseek(fp, 0, SEEK_SET);
      return std::make_unique<StdFile>(fp, size);
    }

    std::fclose(fp);
  }

  return nullptr;
}

StdFile::StdFile(FILE *fp, const long size)
  : Stream((size_t)size),
    _file(fp) {
}

StdFile::~StdFile() {
  std::fclose(_file);
}

std::error_code StdFile::real_read(size_t size, void *data) {
  const auto ret = std::fread(data, 1, size, _file);
  if (ret == size) {
    return {};
  }

  return std::make_error_code(std::errc::invalid_argument);
}
std::error_code StdFile::real_seek(size_t size) {
  const auto ret = fseek(_file, static_cast<long>(size), SEEK_SET);
  if (ret == 0) {
    return {};
  }
  return std::make_error_code(static_cast<std::errc>(errno));
}
