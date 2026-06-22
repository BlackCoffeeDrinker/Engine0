
#include "Platform.hpp"

namespace {

class Sink : public e00::LoggerSink {
  std::string _file_name;
  std::string _name;

public:
  explicit Sink(std::string file_name, std::string name)
      : _file_name(std::move(file_name)),
        _name(std::move(name)) {
  }

  void log(const e00::detail::LogMessage &msg) override {
    if (const auto f = fopen(_file_name.c_str(), "a"); f) {
      fprintf(f, "[%s] %.*s\n", _name.c_str(), static_cast<int>(msg.payload.size()), msg.payload.data());
      fclose(f);
    }
  }

  void flush() override {
  }
};
}// namespace

namespace platform {
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name) {
  return std::make_unique<Sink>("LOG.txt", name);
}
}// namespace platform
