#pragma once

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
  LoggingSeverity _level{ LoggingSeverity::L_VERBOSE };

public:
  virtual ~LoggerSink() = default;
  virtual void log(const detail::LogMessage &msg) = 0;
  virtual void flush() = 0;

  void set_level(LoggingSeverity log_level) { _level = log_level; }
  [[nodiscard]] LoggingSeverity level() const { return _level; }
  [[nodiscard]] bool should_log(LoggingSeverity msg_level) const { return msg_level >= _level; }
};

class Logger {
  std::unique_ptr<LoggerSink> _unique_sink;

  std::vector<LoggerSink *> _sinks;

  // common implementation for after templated public api has been resolved
  template<typename... Args>
  void log_(const experimental::source_location &loc, LoggingSeverity sev, std::string_view fmt, const Args& ...args) const {
    std::string payload;
    detail::LogMessage log_message{
      loc,
      sev,
      payload,
      std::chrono::system_clock::now()
    };

    // Send it to all interested sinks
    for (auto &sink : _sinks) {
      if (!sink->should_log(sev))
        continue;

      // Make sure we have payload
      if (payload.empty()) {
        payload = fmt_lite::format(fmt, std::forward<const Args&>(args)...);
        log_message.payload = payload;
      }

      // Send it
      sink->log(log_message);
    }
  }

public:
  Logger() = default;

  explicit Logger(std::unique_ptr<LoggerSink> && uniqueSink)
    : _unique_sink(std::move(uniqueSink)) {
    AddSink(_unique_sink.get());
  }

  template<typename... Args>
  void Log(const experimental::source_location &loc, LoggingSeverity sev, std::string_view fmt, const Args& ...args) {
    log_(loc, sev, fmt, std::forward<const Args&>(args)...);
  }

  // Helpers
  template<typename... Args>
  void Verbose(const experimental::source_location &loc, std::string_view fmt, Args ...args) const {
    log_(loc, L_VERBOSE, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Error(const experimental::source_location &loc, std::string_view fmt, Args ...args) const {
    log_(loc, L_ERROR, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Info(const experimental::source_location &loc, std::string_view fmt, Args ...args) const {
    log_(loc, L_INFO, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  void Warning(const experimental::source_location &loc, std::string_view fmt, Args ...args) const {
    log_(loc, L_WARNING, fmt, std::forward<Args>(args)...);
  }


  void AddSink(LoggerSink *sink) {
    const auto it = std::find(_sinks.begin(), _sinks.end(), sink);
    if (it != _sinks.end()) {
      return;
    }
    _sinks.push_back(sink);
  }

  void RemoveSink(LoggerSink *sink) {
    auto it = std::find(_sinks.begin(), _sinks.end(), sink);
    if (it != _sinks.end()) {
      _sinks.erase(it);
    }
  }

  void ClearSinks() {
    _sinks.clear();
  }
};

std::unique_ptr<LoggerSink> CreateSink(const std::string& name);

#define E00_LOG(level, ...) default_logger()->Log(experimental::source_location::current(), level, __VA_ARGS__)

#define E00_VERBOSE(...) E00_LOG(e00::L_VERBOSE, __VA_ARGS__)
#define E00_INFO(...) E00_LOG(e00::L_INFO, __VA_ARGS__)
#define E00_WARNING(...) E00_LOG(e00::L_WARNING, __VA_ARGS__)
#define E00_ERROR(...) E00_LOG(e00::L_ERROR, __VA_ARGS__)

}// namespace e00
