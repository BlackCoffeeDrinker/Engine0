#include "Platform.hpp"

#include "Audio.hpp"
#include "Keyboard.hpp"
#include "Menu.hpp"
#include "Mouse.hpp"
#include "Win31Surface.hpp"
#include "Win32Types.hpp"
#include "WinG.hpp"

#include <chrono>// only used for the e00::Engine::Tick(std::chrono::milliseconds) duration type
#include <cstring>
#include <string>

class e00::PlatformData {
public:
  // GetTickCount() is a native KERNEL32 call: avoid <chrono>/std::chrono::steady_clock,
  // which on this mingw toolchain pulls in Win10+ QueryPerformanceCounter-adjacent and
  // UCRT time helpers (e.g. _time64) that Win32s cannot resolve.
  DWORD lastTick = GetTickCount();
};

namespace win31 {
HINSTANCE g_hInstance = nullptr;
HWND g_hwnd = nullptr;
}// namespace win31

namespace {
constexpr const char *kWindowClassName = "Engine00Win31";
constexpr e00::BitmapSizeType kDefaultWidth = 640;
constexpr e00::BitmapSizeType kDefaultHeight = 480;

using win31::g_hInstance;
using win31::g_hwnd;

bool g_hasFocus = true;
bool g_classRegistered = false;
bool g_quitPosted = false;
e00::Engine *g_activeEngine = nullptr;
std::unique_ptr<platform::Surface> g_mainSurface;
e00::BitmapSizeType g_windowWidth = kDefaultWidth;
e00::BitmapSizeType g_windowHeight = kDefaultHeight;

unsigned long ParseULong(std::string_view text) {
  unsigned long value = 0;
  for (char ch: text) {
    if (ch < '0' || ch > '9') {
      break;
    }
    value = value * 10u + static_cast<unsigned long>(ch - '0');
  }
  return value;
}

LRESULT CALLBACK EngineWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  e00::Engine *engine = g_activeEngine;

  switch (msg) {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      g_quitPosted = true;
      PostQuitMessage(0);
      if (engine) {
        engine->QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
      }
      return 0;

    case WM_ACTIVATE:
      g_hasFocus = (LOWORD(wParam) != WA_INACTIVE);
      return 0;

    case WM_SETFOCUS:
      g_hasFocus = true;
      return 0;

    case WM_KILLFOCUS:
      g_hasFocus = false;
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps{};
      HDC hdc = BeginPaint(hwnd, &ps);
      if (hdc && g_mainSurface) {
        auto *surface = static_cast<win31::Win31Surface *>(g_mainSurface.get());
        RECT rc{};
        GetClientRect(hwnd, &rc);
        surface->Present(hdc, 0, 0, static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top));
      }
      EndPaint(hwnd, &ps);
      return 0;
    }

    case WM_ERASEBKGND:
      return 1;

    case WM_COMMAND:
      if (engine && win31::HandleMenuCommand(*engine, wParam)) {
        return 0;
      }
      break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
      if (engine) {
        win31::HandleKeyboardMessage(*engine, msg, wParam, lParam);
      }
      return 0;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      if (engine) {
        win31::HandleMouseMessage(*engine, hwnd, msg, wParam, lParam);
      }
      return 0;

    default:
      break;
  }

  return DefWindowProcA(hwnd, msg, wParam, lParam);
}

bool RegisterWindowClass() {
  if (g_classRegistered) {
    return true;
  }
  if (!g_hInstance) {
    g_hInstance = GetModuleHandleA(nullptr);
  }

  WNDCLASSA wc{};
  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = EngineWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = LoadIconA(nullptr, MAKEINTRESOURCEA(IDI_APPLICATION));
  wc.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(IDC_ARROW));
  wc.hbrBackground = nullptr;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = kWindowClassName;

  if (!RegisterClassA(&wc)) {
    // Class may already exist from a previous run in the same process.
    // Continue anyway.
  }
  g_classRegistered = true;
  return true;
}

}// namespace

namespace platform {

std::string_view PlatformName() {
  return "Win31";
}

void SetSettings(std::string_view key, std::string_view value) {
  // Called from game.ini before/around Init. Accept width/height hints.
  if (key == "width") {
    g_windowWidth = static_cast<e00::BitmapSizeType>(ParseULong(value));
    if (g_windowWidth == 0) g_windowWidth = kDefaultWidth;
  } else if (key == "height") {
    g_windowHeight = static_cast<e00::BitmapSizeType>(ParseULong(value));
    if (g_windowHeight == 0) g_windowHeight = kDefaultHeight;
  }
}

e00::error_code Init() {
  g_hInstance = GetModuleHandleA(nullptr);
  g_hasFocus = true;
  g_quitPosted = false;

  win31::EnsureWinGLoaded();
  win31::InitAudio();
  win31::InitKeyboard();
  win31::InitMouse();

  if (!RegisterWindowClass()) {
    MessageBoxA(nullptr, "RegisterWindowClass failed!", "ERROR", 0x00000000L);
    return e00::make_error_code(e00::errc::not_supported);
  }

  g_mainSurface = std::make_unique<win31::Win31Surface>(g_windowWidth, g_windowHeight);

  g_hwnd = CreateWindowA(
      kWindowClassName,
      "Engine00",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      static_cast<int>(g_windowWidth) + 16,
      static_cast<int>(g_windowHeight) + 48,
      nullptr, nullptr, g_hInstance, nullptr);

  if (!g_hwnd) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "CreateWindowA failed");
    return e00::make_error_code(e00::errc::not_supported);
  }

  ShowWindow(g_hwnd, SW_SHOWNORMAL);
  UpdateWindow(g_hwnd);

  e00::GetDefaultLogger().Info(
      e00::source_location::current(),
      "Win31 backend ready ({}x{}, WinG={})",
      static_cast<int>(g_windowWidth), static_cast<int>(g_windowHeight),
      static_cast<win31::Win31Surface *>(g_mainSurface.get())->UsingWinG() ? "yes" : "no");

  return {};
}

void Exit() {
  win31::ClearMenu(g_hwnd);
  win31::ShutdownAudio();
  win31::QuitMouse();
  win31::QuitKeyboard();
  win31::UnloadWinG();

  g_mainSurface.reset();

  if (g_hwnd) {
    DestroyWindow(g_hwnd);
    g_hwnd = nullptr;
  }
}

void Yield() {
  Sleep(1);
}

// Win32s has no real preemptive threads usable from Win32s apps in a portable way.
// Cooperative fibers could be added later; for now return InvalidThreadId.
ThreadId CreateThread(Task &&task, size_t /*stack_sz*/) {
  std::ignore = task;
  e00::GetDefaultLogger().Warning(
      e00::source_location::current(),
      "platform::CreateThread is not supported on Win31/Win32s (no real threads)");
  return InvalidThreadId;
}

void SetWindowTitle(e00::Engine & /*engine*/, const std::string_view &windowTitle) {
  if (!g_hwnd) {
    return;
  }
  const std::string title(windowTitle);
  SetWindowTextA(g_hwnd, title.c_str());
  e00::GetDefaultLogger().Info(e00::source_location::current(), "Set window title to: {}", title);
}

bool HasFocus(e00::Engine & /*engine*/) {
  return g_hasFocus;
}

bool InitEngine(e00::Engine &engine) {
  g_activeEngine = &engine;
  engine.SetPlatformData(new e00::PlatformData());
  return true;
}

void QuitEngine(e00::Engine &engine) {
  g_activeEngine = nullptr;
  delete engine.GetPlatformData();
  engine.SetPlatformData(nullptr);
}

void ProcessEvents(e00::Engine &engine) {
  auto *data = engine.GetPlatformData();
  if (!data) {
    return;
  }

  MSG msg{};
  while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      g_quitPosted = true;
      engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
      break;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }

  // Keep mouse state fresh even without move messages.
  if (g_hwnd) {
    win31::PollMouse(engine, g_hwnd);
  }

  win31::PumpAudio();

  const DWORD now = GetTickCount();
  const DWORD elapsed = now - data->lastTick;// unsigned wraparound-safe subtraction
  if (elapsed > 0) {
    const auto delta = std::chrono::milliseconds(elapsed);
    engine.Tick(delta);
    e00::ResourceManager::GlobalResourceManager().Tick(delta);
    data->lastTick = now;
  }

  if (g_quitPosted) {
    engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
  }
}

void ProcessDraw(e00::Engine &engine) {
  if (!g_mainSurface || !g_hwnd) {
    return;
  }

  if (const auto painter = g_mainSurface->BeginDraw()) {
    if (engine.RootWidget()) {
      engine.RootWidget()->Paint(*painter);
    }
  }

  HDC hdc = GetDC(g_hwnd);
  if (hdc) {
    RECT rc{};
    GetClientRect(g_hwnd, &rc);
    static_cast<win31::Win31Surface *>(g_mainSurface.get())
        ->Present(hdc, 0, 0, static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top));
    ReleaseDC(g_hwnd, hdc);
  }

  win31::PumpAudio();
}

Surface &GetMainSurface(e00::Engine & /*engine*/) {
  return *g_mainSurface;
}

Surface &GetMainSurface() {
  return *g_mainSurface;
}

system_clock::time_point system_clock::now() noexcept {
  // GetTickCount() is a native KERNEL32 call: avoid <chrono>/std::chrono::steady_clock,
  // which on this mingw toolchain pulls in Win10+ QueryPerformanceCounter-adjacent and
  // UCRT time helpers (e.g. _time64) that Win32s cannot resolve, and which crashed on
  // real Win32s hardware the very first time it was ever called (e.g. from Logger.hpp).
  return time_point(duration(static_cast<rep>(GetTickCount())));
}

void SetMenu(e00::Engine & /*engine*/, std::span<const MenuItem> items) {
  win31::ApplyMenu(g_hwnd, items);
}

void QueueAudioSamples(std::span<const int16_t> pcmMono16, uint32_t sampleRateHz) {
  win31::QueuePcmMono16(pcmMono16, sampleRateHz);
}

// Optimize is provided by engine/src/Platform.cpp (shared).

}// namespace platform

namespace win31 {
HINSTANCE GetAppInstance() { return g_hInstance; }
HWND GetAppWindow() { return g_hwnd; }
void SetAppInstance(HINSTANCE inst) { g_hInstance = inst; }
}// namespace win31
