#pragma once

#include <utility>

#include "SourceLocation.hpp"

namespace e00 {
enum LoggingSeverity {
  L_VERBOSE = 0,
  L_INFO,
  L_WARNING,
  L_ERROR,
  L_NONE,
};

namespace detail {
struct LogMessage {
  experimental::source_location location;
  LoggingSeverity level;
  std::string_view payload;
  std::chrono::system_clock::time_point time;
};
}// namespace detail

class LoggerSink {
protected:
  // sink log level - default is all
  LoggingSeverity _level{LoggingSeverity::L_VERBOSE};

public:
  virtual ~LoggerSink() = default;
  virtual void log(const detail::LogMessage &msg) = 0;
  virtual void flush() = 0;

  void set_level(LoggingSeverity log_level) { _level = log_level; }
  [[nodiscard]] LoggingSeverity level() const { return _level; }
  [[nodiscard]] bool should_log(LoggingSeverity msg_level) const { return msg_level >= _level; }
};

class Logger {
  std::string _name;
  std::unique_ptr<LoggerSink> _unique_sink;

  // common implementation for after templated public api has been resolved
  template<typename... Args>
  void log_(const experimental::source_location &loc, LoggingSeverity sev, std::string_view fmt, const Args &...args) const {
    std::string payload;
    detail::LogMessage log_message{
        loc,
        sev,
        payload,
        std::chrono::system_clock::now()};

    // Send it to all interested sinks
    //for (auto &sink: _sinks) {
    if (const auto sink = _unique_sink.get(); sink) {
      if (sink->should_log(sev)) {
        // Make sure we have payload; don't evaluate it until we know a sink will want the output
        if (payload.empty()) {
          payload = fmt_lite::format(fmt, std::forward<const Args &>(args)...);
          log_message.payload = payload;
        }

        // Send it
        sink->log(log_message);
      }
    }
  }

public:
  explicit Logger(std::string name);
  Logger(const Logger &parentLogger, const std::string &name);

  [[nodiscard]] auto name() const { return _name; }

  template<typename... Args>
  void Log(const experimental::source_location &loc, LoggingSeverity sev, std::string_view fmt, const Args &...args) {
    log_(loc, sev, fmt, std::forward<const Args &>(args)...);
  }

  // Helpers
  template<typename... Args>
  void Verbose(const experimental::source_location &loc, std::string_view fmt, Args... args) const {
    log_(loc, L_VERBOSE, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Error(const experimental::source_location &loc, std::string_view fmt, Args... args) const {
    log_(loc, L_ERROR, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Info(const experimental::source_location &loc, std::string_view fmt, Args... args) const {
    log_(loc, L_INFO, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Warning(const experimental::source_location &loc, std::string_view fmt, Args... args) const {
    log_(loc, L_WARNING, fmt, std::forward<Args>(args)...);
  }
};

Logger &GetDefaultLogger();

}// namespace e00
