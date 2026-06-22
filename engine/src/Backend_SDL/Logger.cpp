
#include "Logger.h"

namespace {
std::unique_ptr<e00::Logger> logger(nullptr);
}

e00::Logger &platform::PlatformLogger() {
  if (!logger) {
    logger = std::make_unique<e00::Logger>(e00::CreateSink("Platform"));
  }

  return *logger;
}
