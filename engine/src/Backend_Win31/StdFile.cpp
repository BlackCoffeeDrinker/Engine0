#include "StdFile.hpp"

#include <string>

namespace platform {

std::unique_ptr<StdFile> StdFile::CreateFromFilename(const std::string_view &fileName, bool writable) {
  const std::string path(fileName);

  const DWORD access = writable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
  const DWORD share = FILE_SHARE_READ;
  const DWORD disposition = writable ? CREATE_ALWAYS : OPEN_EXISTING;

  HANDLE handle = CreateFileA(path.c_str(), access, share, nullptr, disposition,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
  if (handle == INVALID_HANDLE_VALUE || handle == nullptr) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to open file: {}", fileName);
    return nullptr;
  }

  DWORD sizeHigh = 0;
  DWORD sizeLow = GetFileSize(handle, &sizeHigh);
  if (sizeLow == INVALID_FILE_SIZE && GetLastError() != 0) {
    // New/empty writable file is fine
    if (!writable) {
      CloseHandle(handle);
      return nullptr;
    }
    sizeLow = 0;
  }

  // Win32s / our use-case: files under 4GB
  const size_t size = static_cast<size_t>(sizeLow);
  return std::make_unique<StdFile>(handle, size);
}

e00::error_code StdFile::real_write(size_t size, const void *data) {
  DWORD written = 0;
  if (!WriteFile(_handle, data, static_cast<DWORD>(size), &written, nullptr) || written != size) {
    return e00::make_error_code(e00::errc::io_error);
  }
  return {};
}

e00::error_code StdFile::real_read(size_t size, void *data) {
  DWORD read = 0;
  if (!ReadFile(_handle, data, static_cast<DWORD>(size), &read, nullptr) || read != size) {
    return e00::make_error_code(e00::errc::io_error);
  }
  return {};
}

e00::error_code StdFile::real_seek(size_t position) {
  const DWORD rc = SetFilePointer(_handle, static_cast<LONG>(position), nullptr, FILE_BEGIN);
  if (rc == INVALID_FILE_SIZE && GetLastError() != 0) {
    return e00::make_error_code(e00::errc::io_error);
  }
  return {};
}

std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name) {
  return StdFile::CreateFromFilename(name);
}

std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name) {
  return StdFile::CreateFromFilename(name, true);
}

}// namespace platform
