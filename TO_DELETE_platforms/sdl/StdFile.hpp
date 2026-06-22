#pragma once

#include <Engine.hpp>
#include <cstdio>

namespace platform {
class StdFile : public e00::WritableStream {
  FILE *const _file;

public:
  using Stream::Read;

  static std::unique_ptr<StdFile> CreateFromFilename(const std::string_view &fileName, bool writable = false) {
    if (FILE *fp = std::fopen(fileName.data(), writable ? "wb" : "rb")) {
      if (std::fseek(fp, 0, SEEK_END) == 0) {
        const auto size = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);
        return std::make_unique<StdFile>(fp, size);
      }

      std::fclose(fp);
    }

    return nullptr;
  }

  explicit StdFile(FILE *fp, long size)
      : WritableStream(static_cast<size_t>(size)),
        _file(fp) {
  }

  ~StdFile() override {
    fclose(_file);
  }

protected:
  std::error_code real_write(size_t size, const void *data) override {
    if (std::fwrite(data, 1, size, _file) == size) {
      return {};
    }
    return std::make_error_code(static_cast<std::errc>(errno));
  }

  std::error_code real_read(size_t size, void *data) override {
    if (std::fread(data, 1, size, _file) == size) {
      return {};
    }
    return std::make_error_code(static_cast<std::errc>(errno));
  }

  std::error_code real_seek(size_t size) override {
    if (fseek(_file, static_cast<long>(size), SEEK_SET) == 0) {
      return {};
    }
    return std::make_error_code(static_cast<std::errc>(errno));
  }
};
}// namespace platform
