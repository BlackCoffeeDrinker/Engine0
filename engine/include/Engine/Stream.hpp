#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <system_error>

namespace e00 {
/**
 * File data stream
 */
class Stream  {
  const size_t _stream_size;
  size_t _current_position{0};

protected:
  explicit Stream(size_t stream_size)
    : _stream_size(stream_size)
      {}

  virtual std::error_code real_read(size_t size, void *data) = 0;

  virtual std::error_code real_seek(size_t position) = 0;

public:
  struct Iterator {

  };

  Stream(Stream &&other) noexcept = delete;

  Stream(const Stream &other) = delete;

  Stream &operator=(Stream &&) = delete;

  Stream &operator=(const Stream &) = delete;

  virtual ~Stream() = default;

  [[nodiscard]] size_t stream_size() const { return _stream_size; }

  [[nodiscard]] bool end_of_stream() const { return current_position() >= stream_size(); }

  [[nodiscard]] size_t current_position() const { return _current_position; }

  [[nodiscard]] size_t max_read() const { return _stream_size - _current_position; }

  /**
   * Seeks to an absolute position
   *
   * @param new_position the absolute position to seek to
   * @return error code of any error that occurred, if any
   */
  std::error_code seek(size_t new_position) {
    if (new_position <= _stream_size) {
      if (const auto ec = real_seek(new_position)) {
        return ec;
      }
      _current_position = new_position;
      return {};
    }

    return std::make_error_code(std::errc::value_too_large);
  }

  std::error_code read(size_t max_size, void *data) {
    if (max_size > max_read()) {
      return std::make_error_code(std::errc::value_too_large);
    }

    if (max_size == 0) {
      return {};
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
  std::error_code read(std::vector<T> &data) { return read(data.size() * sizeof(T), data.data()); }

  /**
   * Reads until \n
   *
   * @param max_length the max length of the string
   * @return the line
   */
  std::string read_line(size_t max_length = 0xFFFF) {
    if (end_of_stream())
      return {};

    std::string result;

    while (result.length() < max_length) {
      // Exit this loop if we're at EOF
      if (end_of_stream())
        break;

      char c;
      // If we can't read a char, exit
      if (!read(c))
        break;

      // skip returns
      if (c == '\r')
        continue;

      // if we have a new line, quit this loop
      if (c == '\n')
        break;

      result.push_back(c);
    }

    return result;
  }

  /**
   * Read a line into a char buffer
   *
   * @param str the buffer to use
   * @param max_length max length of the buffer
   * @return the buffer, or nullptr if an error occurred
   */
  char *read_line_into(char *str, int max_length) {
    if (max_length <= 0) {
      // Reading less than 0 ?!
      return nullptr;
    }

    // Make our max read (and leave space for the NUL)
    const auto read_max = std::min(max_read(), static_cast<size_t>(max_length - 1));

    // If we don't have anything to read, then exit out
    if (read_max <= 0) {
      // EOF
      return nullptr;
    }

    // Read as much as possible
    if (read(read_max, str)) {
      return nullptr;
    }

    // Find the first \n
    if (const auto *e = memchr(str, '\n', read_max); e != nullptr) {
      // we did find it!
      const auto pos = static_cast<const char*>(e) - str;
      str[pos] = 0;

      // rewind the stream
      (void) seek(current_position() - read_max + pos + 1);
    } else {
      str[read_max + 1] = 0;
    }

    return str;
  }

  template<typename T, unsigned N>
  bool read(T (&t)[N]) {
    return !read(sizeof(T) * N, t);
  }

  template<typename T>
  bool read(T &out) { return !read(sizeof(T), &out); }

  template<typename T>
  std::remove_cv_t<T> read() {
    if (std::remove_cv_t<T> read_value; read(read_value)) {
      return read_value;
    }
    return {};
  }
};
}// namespace e00
