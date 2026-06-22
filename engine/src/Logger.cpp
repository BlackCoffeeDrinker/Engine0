#include "PrivateInclude.hpp"

#include "Platform.hpp"

#include <mutex>

namespace {
std::string make_name(const std::string &parent_name, const std::string &name) {
  return parent_name.empty() ? name : parent_name + "." + name;
}

}// namespace

namespace e00 {
Logger::Logger(std::string name)
    : _name(std::move(name)),
      _unique_sink(platform::CreateSink(name)) {
}

Logger::Logger(const Logger &parentLogger, const std::string &name)
    : _name(make_name(parentLogger._name, name)),
      _unique_sink(platform::CreateSink(_name)) {
}

Logger &GetDefaultLogger() {
  static std::once_flag flag;
  static Logger *default_logger = nullptr;

  std::call_once(flag, []() {
    default_logger = new Logger({});
  });

  return *default_logger;
}
}// namespace e00
