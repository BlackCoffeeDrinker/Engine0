#include "Platform.hpp"
#include "Win32Types.hpp"

#include <string>

namespace {

// Logs every line to LOG.TXT in the current directory. Each call opens the
// file fresh with CreateFileA, appends the line via WriteFile, and closes
// the handle immediately (OPEN_ALWAYS + SetFilePointer(..., FILE_END, ...)),
// rather than keeping a handle open for the process lifetime. This trades a
// bit of per-line overhead for robustness: if the app crashes or is killed,
// the log file is never left in a state where buffered/unflushed writes are
// lost, since nothing is buffered on our side at all.
class LogFileSink : public e00::LoggerSink {
  std::string _name;

public:
  explicit LogFileSink(std::string name) : _name(std::move(name)) {}

  void log(const e00::detail::LogMessage &msg) override {
    const char *level = "I";
    switch (msg.level) {
      case e00::L_VERBOSE: level = "V"; break;
      case e00::L_INFO: level = "I"; break;
      case e00::L_WARNING: level = "W"; break;
      case e00::L_ERROR: level = "E"; break;
      default: level = "?"; break;
    }

    // "[name][L] payload\n"
    std::string line;
    line.reserve(_name.size() + msg.payload.size() + 16);
    line.push_back('[');
    line.append(_name);
    line.append("][");
    line.append(level);
    line.append("] ");
    line.append(msg.payload.data(), msg.payload.size());
    line.push_back('\r');
    line.push_back('\n');

    HANDLE file = CreateFileA("LOG.TXT", GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == nullptr || file == INVALID_HANDLE_VALUE) {
      MessageBoxA(nullptr, "Failed to open log file", "Error", 0x00000000L);
      return;
    }
    SetFilePointer(file, 0, nullptr, FILE_END);
    DWORD written = 0;
    WriteFile(file, line.data(), static_cast<DWORD>(line.size()), &written, nullptr);
    CloseHandle(file);
  }

  void flush() override {}
};

}// namespace

namespace platform {
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name) {
  return std::make_unique<LogFileSink>(name);
}
}// namespace platform
