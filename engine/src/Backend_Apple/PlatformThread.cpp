#include "Platform.hpp"

#include <thread>
#include <vector>

namespace {

class ModernThread {
  std::jthread _native_thread;
};

std::vector<ModernThread> _thread_handles;

}// namespace

namespace platform {

int CreateThread(e00::Task &&task, size_t /*stack_sz*/) {
  // Modern OS can just capture and move the e00::Task right into std::jthread!
  std::jthread native_worker([captured_task = std::move(task)]() mutable {
    captured_task();
  });

  // Detach or track the handle inside your global index array as required
  native_worker.detach();
  return 1;
}

}// namespace platform
