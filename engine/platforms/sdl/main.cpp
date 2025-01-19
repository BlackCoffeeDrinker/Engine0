#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <Platform.hpp>
#include <SDL.h>

#include "SDL_events.h"
#include "SDL_thread.h"

#include "SdlBitmap.hpp"
#include "StdFile.hpp"

namespace {
bool hasFocus;
SDL_Window *sdlWindow;

class SDL_KeyboardSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override {
    return "Keyboard";
  }

  [[nodiscard]] std::string name(uint16_t value) const override {
    return {SDL_GetKeyName(value)};
  }
};

constexpr SDL_KeyboardSystem keyboard_system{};

e00::InputEvent MakeSDLKey(const SDL_KeyboardEvent &key_event) {
  return {keyboard_system, static_cast<uint16_t>(key_event.keysym.sym & 0xFFFF)};
}

e00::Logger &PlatformLogger() {
  static std::unique_ptr<e00::Logger> logger(nullptr);
  if (!logger) {
    logger = std::make_unique<e00::Logger>(e00::CreateSink("Platform"));
  }

  return *logger;
}
}// namespace


namespace platform {
std::string_view PlatformName() {
  return "SDL";
}

void SetSettings(std::string_view key,
                 std::string_view value) {
  PlatformLogger().Info(e00::source_location::current(), "Setting {} = {}", key, value);
  if (key == "width") {
    auto windowWidth = (int) strtol(value.data(), nullptr, 10);
    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);
    SDL_SetWindowSize(sdlWindow, windowWidth, h);
  } else if (key == "height") {
    auto windowHeight = (int) strtol(value.data(), nullptr, 10);
    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);
    SDL_SetWindowSize(sdlWindow, w, windowHeight);
  }
}

std::error_code Init() {
  hasFocus = true;

#ifdef WIN32
  AllocConsole();

  {
    FILE *newStdout = nullptr;
    FILE *newStderr = nullptr;
    FILE *newStdin = nullptr;

    (void) ::freopen_s(&newStdout, "CONOUT$", "w", (__acrt_iob_func(1)));
    (void) ::freopen_s(&newStderr, "CONOUT$", "w", (__acrt_iob_func(2)));
    (void) ::freopen_s(&newStdin, "CONIN$", "r", (__acrt_iob_func(0)));

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    std::wcout.clear();
    std::wcerr.clear();
    std::wcin.clear();
  }
#endif

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return std::make_error_code(std::errc::bad_file_descriptor);
  }

  sdlWindow = SDL_CreateWindow(
      "Engine0",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      640, 480,
      SDL_WINDOW_SHOWN);

  return {};
}

void Exit() {
  if (sdlWindow) {
    SDL_DestroyWindow(sdlWindow);
  }

  SDL_Quit();
}

std::unique_ptr<e00::Stream> OpenStream(const std::string &name) {
  return e00::platform::StdFile::CreateFromFilename("resources/" + name);
}

void ProcessEvents(e00::Engine &engine) {
  SDL_Event e;
  if (SDL_PollEvent(&e) == 0)
    return;

  switch (static_cast<SDL_EventType>(e.type)) {
    default:
      break;

    case SDL_QUIT:
      engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
      break;

    case SDL_WINDOWEVENT:
      switch (static_cast<SDL_WindowEventID>(e.window.event)) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
          hasFocus = true;
          SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
          break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
          hasFocus = false;
          SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
          break;

        default:
          break;
      }
      break;

    case SDL_KEYDOWN:
      engine.ProcessInputEvent(MakeSDLKey(e.key));
      break;

    case SDL_KEYUP:
      break;
  }
}

void ProcessDraw(e00::Engine &engine) {
  if (sdlWindow) {
    SDL_UpdateWindowSurface(sdlWindow);
  }
}

void SetWindowTitle(const std::string_view &newTitle) {
  if (sdlWindow) {
    SDL_SetWindowTitle(sdlWindow, newTitle.data());
  }
}

bool HasFocus() {
  return hasFocus;
}

void Yield() {
  // Yield our timeslice
  SDL_Delay(0);
}
}// namespace platform
