
#include "Platform.hpp"

namespace platform {
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name) {
  class Sink : public e00::LoggerSink {
    std::string _name;

  public:
    explicit Sink(std::string name) : _name(std::move(name)) {
    }

    void log(const e00::detail::LogMessage &msg) override {
      fprintf(stderr, "[%s] %s\n", _name.c_str(), msg.payload.data());
      fflush(stderr);
    }

    void flush() override {
      fflush(stderr);
    }
  };
  return std::make_unique<Sink>(name);
}
}
