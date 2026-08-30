#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

namespace platform {

class StdFile : public e00::WritableStream {
  HANDLE _handle;

public:
  using Stream::Read;

  static std::unique_ptr<StdFile> CreateFromFilename(const std::string_view &fileName, bool writable = false);

  explicit StdFile(HANDLE handle, size_t size)
      : WritableStream(size),
        _handle(handle) {}

  ~StdFile() override {
    if (_handle && _handle != INVALID_HANDLE_VALUE) {
      CloseHandle(_handle);
      _handle = INVALID_HANDLE_VALUE;
    }
  }

protected:
  e00::error_code real_write(size_t size, const void *data) override;
  e00::error_code real_read(size_t size, void *data) override;
  e00::error_code real_seek(size_t position) override;
};

}// namespace platform
