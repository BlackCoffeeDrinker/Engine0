
#pragma once

#include <Engine.hpp>

namespace platform {
using ThreadId = int;
constexpr ThreadId InvalidThreadId = -1;

class Task {
  using InvokeFn = void (*)(void *);
  using DeleteFn = void (*)(void *);
  using MoveFn = void (*)(void *, void *);// Required to move stack contexts safely

  // 16 bytes allows holding up to 4 pointers/32-bit fields on a 386
  static constexpr size_t SOO_BUFFER_SIZE = 16;

  // Ensure strict 4-byte alignment on 386 for optimal bus access
  alignas(void *) char _soo_buffer[SOO_BUFFER_SIZE];

  void *_context = nullptr;
  InvokeFn _invoke = nullptr;
  DeleteFn _delete = nullptr;
  MoveFn _move = nullptr;

  [[nodiscard]] bool IsUsingSOO() const noexcept {
    return _context == static_cast<const void *>(_soo_buffer);
  }

  void Clear() noexcept {
    if (_delete && _context) {
      _delete(_context);
    }
    _context = nullptr;
    _invoke = nullptr;
    _delete = nullptr;
    _move = nullptr;
  }

public:
  Task() noexcept = default;
  ~Task() { Clear(); }

  // Disable copying
  Task(const Task &) = delete;
  Task &operator=(const Task &) = delete;

  // Move Constructor
  Task(Task &&other) noexcept {
    _invoke = other._invoke;
    _delete = other._delete;
    _move = other._move;

    if (other.IsUsingSOO()) {
      _context = _soo_buffer;
      if (other._move) {
        // Safely migrate the type-erased object state into our local buffer
        other._move(other._context, _context);
      }
    } else {
      _context = other._context;
    }

    other._context = nullptr;
    other._invoke = nullptr;
    other._delete = nullptr;
    other._move = nullptr;
  }

  // Move Assignment Operator
  Task &operator=(Task &&other) noexcept {
    if (this != &other) {
      Clear();

      _invoke = other._invoke;
      _delete = other._delete;
      _move = other._move;

      if (other.IsUsingSOO()) {
        _context = _soo_buffer;
        if (other._move) {
          other._move(other._context, _context);
        }
      } else {
        _context = other._context;
      }

      other._context = nullptr;
      other._invoke = nullptr;
      other._delete = nullptr;
      other._move = nullptr;
    }
    return *this;
  }

  // Templated Constructor with conditional compile-time SOO routing
  template<typename F>
  Task(F &&callable) {
    using DecayedF = std::decay_t<F>;

    constexpr bool fits_in_soo = sizeof(DecayedF) <= SOO_BUFFER_SIZE;
    constexpr bool aligns_safely = alignof(DecayedF) <= alignof(void *);

    if constexpr (fits_in_soo && aligns_safely) {
      // Allocate directly inside the pre-allocated struct memory
      _context = _soo_buffer;
      ::new (_context) DecayedF(std::forward<F>(callable));

      _delete = [](void *ctx) {
        static_cast<DecayedF *>(ctx)->~DecayedF();
      };

      _move = [](void *src, void *dst) {
        ::new (dst) DecayedF(std::move(*static_cast<DecayedF *>(src)));
        static_cast<DecayedF *>(src)->~DecayedF();
      };
    } else {
      // Fallback to heap allocation for large capture blocks
      _context = new DecayedF(std::forward<F>(callable));

      _delete = [](void *ctx) {
        delete static_cast<DecayedF *>(ctx);
      };

      _move = nullptr;// Heap-allocated pointer copies don't need a move shim
    }

    _invoke = [](void *ctx) {
      (*static_cast<DecayedF *>(ctx))();
    };
  }

  explicit operator bool() const noexcept { return _invoke != nullptr; }

  void operator()() const {
    if (_invoke && _context) {
      _invoke(_context);
    }
  }
};

enum class MemoryPlacement {
  VideoMemOnly,
  SystemMemory
};

class Surface : public e00::DrawableSurface {
public:
  virtual void SetPalette(const e00::FixedPalette &palette) = 0;

  virtual std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, MemoryPlacement where) = 0;
};

std::string_view PlatformName();

void SetSettings(std::string_view key,
                 std::string_view value);

std::error_code Init();
void Exit();
void Yield();

/**
 * Boots a background system thread or fiber using a modern move-only container.
 * 
 * @param task A move-only callable containing lambdas with move-captured resources.
 * @param stack_sz The stack size in bytes (strictly enforced on DOS, ignored elsewhere).
 * @return An integer thread ID on success, or -1 on failure.
 */
ThreadId CreateThread(Task &&task, size_t stack_sz = 16384);

void SetWindowTitle(e00::Engine &engine, const std::string_view &windowTitle);

bool HasFocus(e00::Engine &engine);
bool InitEngine(e00::Engine &engine);
void ProcessEvents(e00::Engine &engine);
void ProcessDraw(e00::Engine &engine);
void QuitEngine(e00::Engine &engine);

// Open a raw file on disk
std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name);
std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name);

// Logger Sink
std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name);

// Make a hardware surface
Surface &GetMainSurface(e00::Engine &engine);
Surface &GetMainSurface();

/**
 * Takes any generic loaded bitmap source, converts its layout/color-depth to 
 * match the active Main Surface exactly, and bakes it into an optimized surface.
 * 
 * @param source The raw loaded bitmap asset (from a PNG, GIF, BMP, etc.)
 * @param preferHW If true, attempts to allocate this inside high-speed off-screen VRAM.
 * @return A surface that matches the main screen format perfectly.
 */
std::unique_ptr<e00::DrawableSurface> Optimize(const e00::DrawableSurface &source, bool preferHW);
}// namespace platform
