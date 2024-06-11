#include <cstdio>
#include <crt0.h>
#include <dpmi.h>
#include <csignal>

#include "Engine.hpp"
#include "DosDetect.hpp"
#include "DosMode13.hpp"
#include "DosModeVesa.hpp"
#include "MouseInt33/MouseComDrv.hpp"
#include "KeyboardDecoding/KeyboardEventHandler.hpp"

#include "Signals.hpp"
#include "Timer.hpp"

int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY | _CRT0_FLAG_NONMOVE_SBRK;

namespace {
void signal_handler(int sig) {
  raise(sig);
}

class LoggerSink : public e00::log::Sink {
public:
  void info(const std::string_view &string) override {
    puts(string.data());
  }
  void error(const std::string_view &string) override {
    fputs(string.data(), stderr);
    fputc('\n', stderr);
  }
};
}// namespace

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  // Early Init
  LoggerSink logger_sink;
  e00::Engine::add_logger_sink(&logger_sink);

  std::puts("Initialize System: ");
  SystemInfo info = guessSystemInfo();
  switch (info.type) {
    case SystemType::UNSUPPORTED: break;
    case SystemType::DOS: std::puts("DOS"); break;
    case SystemType::WINDOWS_3X: std::puts("Windows 3.X"); break;
    case SystemType::WINDOWS_95: std::puts("Windows 95"); break;
    case SystemType::WINDOWS_98: std::puts("Windows 98"); break;
    case SystemType::WINDOWS_ME: std::puts("Windows ME"); break;
    case SystemType::WINDOWS_NT: std::puts("Windows NT"); break;
    case SystemType::STEALTH_WINDOWS: std::puts("Windows (Stealth)"); break;
    case SystemType::OS2_WARP: std::puts("OS/2 Warp"); break;
    case SystemType::OS2: std::puts("OS/2"); break;
    case SystemType::DOSEMU: std::puts("DOSEmu"); break;
    case SystemType::OPENDOS: std::puts("OpenDOS"); break;
    case SystemType::OPENDOS_EMM386: std::puts("OpenDOS EMM386"); break;
  }

  signals_init(&signal_handler);

  // Video init
  const auto video_info = video_get_info();
  if (video_info.ega_info.present) {
    std::puts("Found EGA");
  }
  if (video_info.vga_info.present) {
    std::puts("Found VGA");
  }

  //  vesa_init();

  auto engine = CreateGameEngine({ "res/" });
  if (auto ec = engine->Init()) {
    std::puts("Failed to init engine");
    std::puts(ec.message().c_str());
    return -1;
  }

  e00::InputEvent esc(e00::InputEvent::Type::Release, KeyboardInputSystem(), Key::KEY_ESC);

  std::puts("==================================================");
  std::puts(engine->Name().data());
  std::puts("==================================================");

  const auto v = engine->Actions();
  for (const auto &i : v) {
    std::printf("Action: (%s/%p) %s/%d\n", i.category().name().data(), &(i.category()), i.message().data(), i.value());
  }

  engine->BindInputEventToAction(e00::Engine::BuiltInAction_Quit(), esc);

  std::puts("Press ENTER to continue");
  std::getchar();

  timer_init();

  KeyboardEventHandler keyboard_event_handler;

  MouseComDrv mouse_drv;
  if (mouse_drv.has_mouse()) {
    std::puts("Initialize Mouse: Mouse found");
  } else {
    std::puts("Initialize Mouse: Mouse not found");
  }

  auto current_time = timer_since_start();

  // who cares about anything, set vga 320x200 (x400?) 8bit
  //  auto screen = video_get_screen();

  while (engine->IsRunning()) {
    const auto tss = timer_since_start();
    mouse_drv.mouse_update();
    __dpmi_yield();

    const auto kbd = keyboard_event_handler.event();
    const auto mse = mouse_drv.event_data();

    if (kbd) {
      engine->ProcessInputEvent(kbd);
    }
    if (mse) {
      engine->ProcessInputEvent(mse);
    }

    engine->Tick(tss - current_time);
    // engine->Draw(screen);

    current_time = tss;
  }

  //  delete screen;

  timer_shutdown();
  signals_shutdown();

  return 0;
}
