
#include "PlatformThread.hpp"

#include <csetjmp>

namespace dos {
enum class ThreadState { FREE,
                         READY,
                         RUNNING,
                         BLOCKED,
                         FINISHED };

// Add a Task storage holder directly to your DOS_ThreadContext definition
struct DOS_ThreadContext {
  std::jmp_buf env;
  ThreadState state;
  int id;
  void *stack_base;
  size_t stack_size;

  // The modern container sits directly inside your pre-allocated array!
  platform::Task bound_task;
};

constexpr int DOS_MAX_THREADS = 8;// Main, Asset Loader, Audio Streamer, etc.
static std::array<DOS_ThreadContext, DOS_MAX_THREADS> threads;
static int current_thread = 0;


static void DOS_ThreadTrampoline() {
  auto &ctx = threads[current_thread];

  // Execute the type-erased task wrapper directly
  if (ctx.bound_task) {
    ctx.bound_task();
  }

  ctx.state = ThreadState::FINISHED;
  while (true) { platform::Yield(); }
}
}// namespace dos

namespace platform {

int CreateThread(Task &&task, size_t stack_sz) {
  for (int i = 1; i < dos::threads.size(); ++i) {
    auto &ctx = dos::threads[i];

    if (ctx.state == dos::ThreadState::FREE) {
      ctx.id = i;
      ctx.stack_size = stack_sz;
      ctx.stack_base = std::malloc(stack_sz);

      ctx.bound_task = std::move(task);
      ctx.state = dos::ThreadState::READY;

      if (setjmp(ctx.env) == 0) {
        auto *raw_buf = reinterpret_cast<struct std::__jmp_buf *>(ctx.env);
        uintptr_t stack_top = reinterpret_cast<uintptr_t>(ctx.stack_base) + stack_sz;
        stack_top &= ~3U;

        raw_buf->__esp = stack_top;
        raw_buf->__ebp = stack_top;
        raw_buf->__eip = reinterpret_cast<unsigned long>(&dos::DOS_ThreadTrampoline);
      }
      return i;
    }
  }

  return InvalidThreadId;
}

}// namespace platform
