#pragma once

#include <cstring>
#include <vector>

namespace e00 {
/**
 * A class representing an abstraction for data streams.
 *
 * The Stream class provides functionality for reading data streams.
 * The backing may or may not be a file; it could be a "file" in a ZIP
 * or a TAR
 */
class Stream {
protected:
  size_t _stream_size;
  size_t _current_position{0};

  explicit Stream(size_t stream_size)
      : _stream_size(stream_size) {}

  virtual std::error_code real_read(size_t size, void *data) = 0;

  virtual std::error_code real_seek(size_t position) = 0;

public:
  Stream(Stream &&other) noexcept = delete;

  Stream(const Stream &other) = delete;

  Stream &operator=(Stream &&) = delete;

  Stream &operator=(const Stream &) = delete;

  virtual ~Stream() = default;

  /**
   * Represents the size or length of a data stream.
   *
   * The stream_size constant is used to define or indicate
   * the total size of a data stream.
   */
  [[nodiscard]] size_t Size() const { return _stream_size; }

  /**
   * Checks if the stream has reached the end.
   *
   * This method determines if the current position in the stream
   * has reached or surpassed the total size of the stream.
   *
   * @return true if the current position is at or beyond the end of the stream,
   *         false otherwise.
   */
  [[nodiscard]] bool AtEnd() const { return Position() >= Size(); }

  /**
   * Current position in the stream
   * 
   * @return the position in the stream
   */
  [[nodiscard]] size_t Position() const { return _current_position; }

  /**
   * 
   * @return maximum bytes that can be read
   */
  [[nodiscard]] size_t AvailableToRead() const { return _stream_size - _current_position; }

  /**
   * Seeks to an absolute position
   *
   * @param new_position the absolute position to seek to
   * @return error code of any error that occurred, if any
   */
  std::error_code SeekTo(size_t new_position) {
    if (new_position <= _stream_size) {
      if (const auto ec = real_seek(new_position)) {
        return ec;
      }
      _current_position = new_position;
      return {};
    }

    return std::make_error_code(std::errc::value_too_large);
  }

  /**
   * Reads data from the stream.
   * 
   * @param data A pointer to the buffer where the read data will be stored.
   * @param max_size The size, in bytes, to read from the stream.
   * @return an error code if any errors occurred
   */
  std::error_code Read(size_t max_size, void *data) {
    if (max_size == 0) {
      return std::make_error_code(std::errc::invalid_argument);
    }

    if (max_size > AvailableToRead()) {
      return std::make_error_code(std::errc::io_error);
    }

    if (const auto ec = real_read(max_size, data)) {
      // reset position
      (void) real_seek(_current_position);
      return ec;
    }

    _current_position += max_size;
    return {};
  }
  

  template<typename T>
  std::error_code Read(std::vector<T> &data) { return Read(data.size() * sizeof(T), data.data()); }

  template<typename T, size_t N>
  std::error_code Read(std::array<T, N> &out) { return Read(out.size() * sizeof(T), out.data()); }
  
  template<typename T>
  std::error_code Read(T &out) { return Read(sizeof(T), &out); }
  
  /**
   * Read a line into a char buffer
   *
   * @param str the buffer to use
   * @param max_length max length of the buffer
   * @return the buffer, or nullptr if an error occurred
   */
  char *ReadLineInto(char *str, int max_length) {
    if (max_length <= 0) {
      // Reading less than 0?
      return nullptr;
    }

    // Make our max read (and leave space for the NUL)
    const auto read_max = std::min(AvailableToRead(), static_cast<size_t>(max_length - 1));

    // If we do not have anything to read, then exit out
    if (read_max <= 0) {
      // EOF
      return nullptr;
    }

    // Read as much as possible
    if (Read(read_max, str)) {
      return nullptr;
    }

    // Find the first \n
    if (const auto *e = memchr(str, '\n', read_max); e != nullptr) {
      // we did find a new line
      const auto pos = static_cast<const char *>(e) - str;
      str[pos] = 0;

      // rewind the stream
      (void) SeekTo(Position() - read_max + pos + 1);
    } else {
      str[read_max] = 0;
    }

    return str;
  }
};

/**
 * A class representing a writable data stream.
 *
 * The WritableStream class provides functionality for writing data to a stream.
 * It is designed for handling write operations and managing the flow of data
 * to a specific output destination.
 */
class WritableStream : public Stream {
protected:
  explicit WritableStream(size_t stream_size)
      : Stream(stream_size) {}

  virtual std::error_code real_write(size_t size, const void *data) = 0;

public:
  ~WritableStream() override = default;

  std::error_code Write(size_t size, const void *data) {
    if (size == 0) {
      return {};
    }

    if (const auto ec = real_write(size, data)) {
      // reset position
      (void) real_seek(_current_position);
      return ec;
    }

    if (_current_position + size > _stream_size) {
      _stream_size = _current_position + size;
    }

    _current_position += size;
    return {};
  }

  template<typename T, size_t N>
  std::error_code Write(const std::array<T, N> &data) { return Write(data.size() * sizeof(T), data.data()); }

  template<typename T>
  std::error_code Write(const std::vector<T> &data) { return Write(data.size() * sizeof(T), data.data()); }

  template<typename T>
  bool WriteLittleEndian(const T &data) { return !Write(sizeof(T), &data); }
};
}// namespace e00
