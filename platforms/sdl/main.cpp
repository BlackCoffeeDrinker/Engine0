#ifdef WIN32
#include <windows.h>
#endif

#include <memory>
#include <string>
#include <iostream>

#include <Engine.hpp>
#include <utility>
#include <SDL.h>

#include "StdFile.hpp"
#include "SDL_events.h"
#include "SDL_thread.h"

namespace {
const auto OPTIMAL_RENDER_DELAY = std::chrono::milliseconds(32);

class SDL_KeyboardSystem : public e00::InputSystem {
public:
  [[nodiscard]] std::string name() const override {
    return "Keyboard";
  }

  [[nodiscard]] std::string name(uint16_t value) const override {
    return { SDL_GetKeyName(value) };
  }
};

class SDLBitmap : public e00::Bitmap {
  SDL_Surface *_surface;

public:
  NOT_COPYABLE(SDLBitmap);

  explicit SDLBitmap(SDL_Surface *fromSurface) : _surface(fromSurface) {
  }

  SDLBitmap(int w, int h)
    : _surface(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0)) {
  }

  SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp)
    : _surface(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0)) {
  }

  ~SDLBitmap() override {
    SDL_FreeSurface(_surface);
  }

  Type GetType() const override {
    return Type::SOFTWARE;
  }

  BitDepth GetBitDepth() const override {
    return BitDepth::TRUE_COLOR_888;
  }

  [[nodiscard]] auto Surface() const { return _surface; }

  [[nodiscard]] e00::Vec2D<uint16_t> Size() const override {
    if (_surface) {
      return {
        static_cast<uint16_t>(_surface->w),
        static_cast<uint16_t>(_surface->h)
      };
    }

    return {};
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return {};
  }

  void SetPixel(const e00::Vec2D<uint16_t> &position, const e00::Color &color) override {
  }
};

const SDL_KeyboardSystem keyboard_system{};

e00::InputEvent MakeSDLKey(const SDL_KeyboardEvent &key_event) {
  return { keyboard_system, static_cast<uint16_t>(key_event.keysym.sym & 0xFFFF) };
}

}// namespace

namespace platform {

std::unique_ptr<e00::Bitmap> CreatePlatformBitmap(e00::Vec2D<uint16_t> size, e00::Bitmap::BitDepth bpp) {
  return std::make_unique<SDLBitmap>(size.x, size.y, bpp);
}

}// namespace platform

static void EventLoop(e00::Engine &engine, bool& hadFocus) {
  SDL_Event e;
  if (SDL_PollEvent(&e) != 0) {
    const auto eventType = static_cast<SDL_EventType>(e.type);
    switch (eventType) {
      default: break;

      case SDL_QUIT:
        engine.ExecuteAction(e00::Engine::BuiltInAction_Quit());
        break;

      case SDL_WINDOWEVENT:
        switch (static_cast<SDL_WindowEventID>(e.window.event)) {
          case SDL_WINDOWEVENT_FOCUS_GAINED:
            hadFocus = true;
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_NORMAL);
            break;
          case SDL_WINDOWEVENT_FOCUS_LOST:
            hadFocus = false;
            SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);
            break;

          default: break;
        }
        break;

      case SDL_KEYDOWN:
        engine.ProcessInputEvent(MakeSDLKey(e.key));
        break;

      case SDL_KEYUP:
        break;
    }
  }
}

static void MainLoop(e00::Engine &engine, SDL_Window *sdlWindow) {
  auto oldTime = std::chrono::steady_clock::now();
  auto lastRenderTime = std::chrono::steady_clock::now();
  bool hadFocus = true;

  while (engine.IsRunning()) {
    auto nowPoint = std::chrono::steady_clock::now();
    const auto endPoint = nowPoint + OPTIMAL_RENDER_DELAY;

    // Compute delta
    const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(nowPoint - oldTime);
    engine.Tick(delta);
    oldTime = nowPoint;

    // Event loop
    while (std::chrono::steady_clock::now() < endPoint) {
      EventLoop(engine, hadFocus);
    }

    // Render
    if (hadFocus) {
      // make sure we have the optimal tick rate
      if (std::chrono::duration_cast<std::remove_cv_t<decltype(OPTIMAL_RENDER_DELAY)>>(nowPoint - lastRenderTime) > OPTIMAL_RENDER_DELAY) {
        engine.Draw();
        SDL_UpdateWindowSurface(sdlWindow);
      }
    }

    // Don't steal every cycle if we're not focused
    if (!hadFocus) {
      // Yield our timeslice
      SDL_Delay(0);
    }
  }
}

#ifdef WIN32
INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, PWSTR pCmdLine, int nCmdShow) {
  AllocConsole();

  {
    FILE *newStdout = nullptr;
    FILE *newStderr = nullptr;
    FILE *newStdin = nullptr;

    (void)::freopen_s(&newStdout, "CONOUT$", "w", (__acrt_iob_func(1)));
    (void)::freopen_s(&newStderr, "CONOUT$", "w", (__acrt_iob_func(2)));
    (void)::freopen_s(&newStdin, "CONIN$", "r", (__acrt_iob_func(0)));

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    std::wcout.clear();
    std::wcerr.clear();
    std::wcin.clear();
  }
#else
int main(int, char **) {
#endif
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  auto engine = CreateGameEngine();
  if (!engine) {
    SDL_Quit();
    printf("Unable to start engine");
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow(
    engine->Name().data(),
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    640,
    480,
    SDL_WINDOW_ALLOW_HIGHDPI);
  //platform::_screen_bitmap = std::make_unique<SDLBitmap>(SDL_GetWindowSurface(window));

  if (const auto ec = engine->Init()) {
    printf("Error: %s\n", ec.message().data());
    SDL_Quit();
    return -1;
  }

  /*  {
      e00::Color colors[] = { { 142, 197, 181 }, { 112, 173, 164 }, { 91, 154, 154 }, { 77, 125, 141 }, { 54, 84, 113 }, { 42, 66, 104 }, { 25, 36, 73 }, { 21, 13, 55 }, { 20, 7, 38 }, { 12, 22, 52 }, { 16, 45, 64 }, { 22, 68, 77 }, { 31, 96, 94 }, { 46, 129, 114 }, { 64, 158, 116 }, { 98, 187, 125 }, { 121, 205, 122 }, { 36, 13, 57 }, { 56, 21, 78 }, { 85, 30, 102 }, { 113, 45, 122 }, { 146, 66, 143 }, { 173, 96, 143 }, { 146, 181, 184 }, { 134, 157, 162 }, { 121, 135, 142 }, { 103, 108, 120 }, { 89, 89, 102 }, { 71, 68, 78 }, { 63, 58, 67 }, { 54, 47, 55 }, { 42, 35, 40 }, { 14, 2, 10 }, { 52, 15, 33 }, { 78, 31, 50 }, { 116, 52, 68 }, { 140, 69, 78 }, { 162, 89, 89 }, { 179, 115, 106 }, { 198, 149, 128 }, { 210, 174, 145 }, { 229, 204, 171 }, { 242, 233, 205 }, { 242, 223, 167 }, { 223, 182, 132 }, { 210, 149, 103 }, { 191, 110, 80 }, { 178, 78, 61 }, { 164, 48, 69 }, { 149, 36, 82 } };

      //_system->SetPalette(colors);
    }*/

  MainLoop(*engine, window);

  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}