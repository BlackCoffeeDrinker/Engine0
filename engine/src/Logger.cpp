#include "PrivateInclude.hpp"
#include <iostream>

namespace {
class DefaultSink : public e00::LoggerSink {

  static std::string_view SeverityToString(e00::LoggingSeverity sev) {
    switch (sev) {
      case e00::L_VERBOSE: return "V";
      case e00::L_INFO: return "I";
      case e00::L_WARNING: return "W";
      case e00::L_ERROR: return "E";
      case e00::L_NONE: break;
    }

    return "";
  }

public:
  void log(const e00::detail::LogMessage &msg) override {
    std::cerr
      << msg.location.file_name() << ":" << msg.location.line()
      << " [" << SeverityToString(msg.level)
      << "] " << msg.time.time_since_epoch().count()
      << ": " << msg.payload << "\n";
  }

  void flush() override {
  }

};

class E00Logger : public e00::Logger {
  DefaultSink _defaultSink;

public:
  E00Logger() : e00::Logger() {
    AddSink(&_defaultSink);
  }
};
}

