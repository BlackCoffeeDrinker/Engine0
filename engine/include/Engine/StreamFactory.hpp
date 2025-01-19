#pragma once

namespace e00 {
class StreamFactory {
  StreamFactory();

public:
  ~StreamFactory();

  static StreamFactory &GlobalStreamFactory();

  /**
   * Finds a stream of name
   *
   * @param name
   * @return
   */
  std::unique_ptr<Stream> OpenStream(const std::string &name);
};
}// namespace e00
