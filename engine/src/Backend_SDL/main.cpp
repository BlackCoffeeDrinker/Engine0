
#include "Bitmap.hpp"
#include "Logger.h"
#include "Platform.hpp"
#include "SDL_KeyboardSystem.h"
#include "StdFile.hpp"

#include <chrono>

class e00::PlatformData {
public:
  std::chrono::steady_clock::time_point lastTick = std::chrono::steady_clock::now();
};

#include <SDL3/SDL.h>


namespace {
bool hasFocus;
SDL_Window *sdlWindow;
std::unique_ptr<platform::Surface> mainSurface;
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

//  SDL_SetHint(SDL_HINT_MAC_, "0");
  SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "1");
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS )) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return std::make_error_code(std::errc::bad_file_descriptor);
  }

  sdlWindow = SDL_CreateWindow(
      "Engine0",
      640, 480,
      SDL_WINDOW_RESIZABLE);

  mainSurface = std::make_unique<SDLBitmap>(SDL_GetWindowSurface(sdlWindow));

  return {};
}

void Exit() {
  if (sdlWindow) {
    SDL_DestroyWindow(sdlWindow);
  }

  mainSurface.reset();

  SDL_Quit();
}

std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name) {
  return StdFile::CreateFromFilename(name);
}

std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name) {
  return StdFile::CreateFromFilename(name, true);
}

void ProcessEvents(e00::Engine &engine) {
  auto *data = engine.GetPlatformData();
  if (!data) return;

  auto now = std::chrono::steady_clock::now();
  auto nextFrameTime = data->lastTick + std::chrono::milliseconds(16);

  SDL_Event e;
  if (now < nextFrameTime) {
    auto waitTime = std::chrono::duration_cast<std::chrono::milliseconds>(nextFrameTime - now).count();
    if (SDL_WaitEventTimeout(&e, (int)waitTime)) {
       do {
        switch (static_cast<SDL_EventType>(e.type)) {
          default:
            break;

          case SDL_EVENT_QUIT:
            engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
            break;

          case SDL_EVENT_WINDOW_FOCUS_GAINED:
            hasFocus = true;
            SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
            break;

          case SDL_EVENT_WINDOW_FOCUS_LOST:
            hasFocus = false;
            SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_LOW);
            break;

          case SDL_EVENT_KEY_DOWN:
            engine.ProcessInputEvent(MakeSDLKey(e.key));
            break;

          case SDL_EVENT_KEY_UP:
            break;
        }
      } while (SDL_PollEvent(&e));
    }
  } else {
    while (SDL_PollEvent(&e)) {
      switch (static_cast<SDL_EventType>(e.type)) {
        default:
          break;

        case SDL_EVENT_QUIT:
          engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
          break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          hasFocus = true;
          SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
          break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
          hasFocus = false;
          SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_LOW);
          break;

        case SDL_EVENT_KEY_DOWN:
          engine.ProcessInputEvent(MakeSDLKey(e.key));
          break;

        case SDL_EVENT_KEY_UP:
          break;
      }
    }
  }

  now = std::chrono::steady_clock::now();
  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - data->lastTick);
  if (delta.count() > 0) {
    engine.Tick(delta);
    e00::ResourceManager::GlobalResourceManager().Tick(delta);
    data->lastTick += delta;
  }
}

void ProcessDraw(e00::Engine &engine) {
  if (sdlWindow) {
    auto painter = GetMainSurface(engine).BeginDraw();
    engine.RootWidget()->Paint(*painter);
    SDL_UpdateWindowSurface(sdlWindow);
  }
}

bool InitEngine(e00::Engine &engine) {
  engine.SetPlatformData(new e00::PlatformData());
  return true;
}

void QuitEngine(e00::Engine &engine) {
  delete engine.GetPlatformData();
  engine.SetPlatformData(nullptr);
}

void SetWindowTitle(e00::Engine &engine, const std::string_view &newTitle) {
  if (sdlWindow) {
    SDL_SetWindowTitle(sdlWindow, newTitle.data());
  }
}

Surface &GetMainSurface(e00::Engine &engine) {
  return *mainSurface;
}

Surface &GetMainSurface() {
  return *mainSurface;
}


bool HasFocus(e00::Engine &engine) {
  return hasFocus;
}

void Yield() {
  // Yield our timeslice
  SDL_Delay(0);
}

}// namespace platform
