#pragma once

#include <catch2/catch_test_macros.hpp>

#include <Engine.hpp>
#include <fstream>

class TestFileStream : public e00::WritableStream {
  FILE *const _file;

public:
  using Stream::Read;

  static std::unique_ptr<TestFileStream> CreateFromFilename(const std::string_view &fileName, bool writeable = false) {
    std::string name(fileName);
    if (FILE *fp = std::fopen(name.c_str(), writeable ? "wb" : "rb")) {
      if (std::fseek(fp, 0, SEEK_END) == 0) {
        const auto size = std::ftell(fp);
        std::fseek(fp, 0, SEEK_SET);
        return std::make_unique<TestFileStream>(fp, size);
      }

      std::fclose(fp);
    }

    return nullptr;
  }

  explicit TestFileStream(FILE *fp, long size)
      : WritableStream(static_cast<size_t>(size)),
        _file(fp) {
  }

  ~TestFileStream() override {
    fclose(_file);
  }

protected:
  std::error_code real_write(size_t size, const void *data) override {
    return std::fwrite(data, 1, size, _file) != size
               ? std::make_error_code(static_cast<std::errc>(errno))
               : std::error_code();
  }

  std::error_code real_read(size_t size, void *data) override {
    return std::fread(data, 1, size, _file) != size
               ? std::make_error_code(static_cast<std::errc>(errno))
               : std::error_code();
  }

  std::error_code real_seek(size_t size) override {
    return std::fseek(_file, static_cast<long>(size), SEEK_SET) != 0
               ? std::make_error_code(static_cast<std::errc>(errno))
               : std::error_code();
  }
};
