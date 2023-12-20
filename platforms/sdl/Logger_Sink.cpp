#include <Engine.hpp>

namespace e00 {

std::unique_ptr<LoggerSink> CreateSink(const std::string &name) {
  class Sink : public LoggerSink {
    std::string _name;

  public:
    explicit Sink(std::string name) : _name(std::move(name)) {
    }

    void log(const detail::LogMessage &msg) override {
      fprintf(stderr, "[%s] %s\n", _name.c_str(), msg.payload.data());
      fflush(stderr);
    }

    void flush() override {
      fflush(stderr);
    }
  };
  return std::make_unique<Sink>(name);
}

}// namespace e00
