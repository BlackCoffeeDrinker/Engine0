
#include "Platform.hpp"

/* these headers only apply to djgpp */
#include <crt0.h>
#include <dpmi.h>
#include <sys/nearptr.h>

#include <memory>

#include "BitmapData.hpp"
#include "OSType.hpp"
#include "Video/Ega.hpp"
#include "Video/VGA_12h.hpp"
#include "keyboard.hpp"
#include "memory.hpp"
#include "mouse.h"
#include "timer.hpp"

namespace {
bool hasNearPtr = false;
std::unique_ptr<platform::Surface> mainSurface;
std::unique_ptr<e00::Bitmap> mainSurfaceBuffer;
DOS::OSInfo osInfo = {};

enum class GraphicsAdapter {
  EGA,
  VGA
};

GraphicsAdapter DetectGraphicsAdapter() {
  __dpmi_regs r = {};

  // Call BIOS Function INT 10h, AX=1A00h (Get Display Combination Code)
  r.x.ax = 0x1A00;
  __dpmi_int(0x10, &r);

  // If AL returns 0x1A, the system supports VGA functionality queries
  if (r.h.al == 0x1A) {
    // BL or BH codes 0x07, 0x08 represent active VGA configurations
    if (r.h.bl == 0x07 || r.h.bl == 0x08) {
      return GraphicsAdapter::VGA;
    }
  }

  // Fallback: If 1A00h is unsupported, use the older EGA verification route (INT 10h, AH=12h, BL=10h)
  std::memset(&r, 0, sizeof(r));
  r.h.ah = 0x12;
  r.h.bl = 0x10;
  __dpmi_int(0x10, &r);

  // If BX returns unmodified or custom parameters, an EGA card is present
  if (r.h.bl != 0x10) {
    return GraphicsAdapter::EGA;
  }

  return GraphicsAdapter::VGA;// Default fallback safety
}
}// namespace

class e00::PlatformData {
public:
  std::chrono::microseconds oldTime = std::chrono::microseconds::zero();
};

namespace platform {
std::string_view PlatformName() {
  if (osInfo.type == DOS::OSType::Type_UNKNOWN) {
    osInfo = DOS::DetectOS();
  }

  switch (osInfo.type) {
    case DOS::OSType::Type_UNKNOWN: return "DOS";
    case DOS::OSType::Type_WIN3: return "DOS (Win3)";
    case DOS::OSType::Type_WIN95: return "DOS (Win95)";
    case DOS::OSType::Type_WIN98: return "DOS (Win98)";
    case DOS::OSType::Type_WINME: return "DOS (WinME)";
    case DOS::OSType::Type_WINNT: return "DOS (WinNT)";
    case DOS::OSType::Type_OS2: return "DOS (OS/2)";
    case DOS::OSType::Type_WARP: return "DOS (Warp)";
    case DOS::OSType::Type_DOSEMU: return "DOS (DOSBox)";
    case DOS::OSType::Type_OPENDOS: return "DOS (OpenDOS)";
    case DOS::OSType::Type_OPENDOS_EMM386: return "DOS (OpenDOS EMM386)";
    case DOS::OSType::Type_DOSBOX_X: return "DOS (DOSBox-X)";
    case DOS::OSType::Type_PURE_DOS: return "DOS (Pure DOS)";
    default: break;
  }

  return "DOS";
}

void SetWindowTitle(e00::Engine &engine, const std::string_view &windowTitle) { std::ignore = windowTitle; }

void SetSettings(std::string_view key,
                 std::string_view value) {
  // Called before init
}

std::error_code Init() {
  /* make sure djgpp won't move our memory around */
  _crt0_startup_flags &= ~_CRT0_FLAG_UNIX_SBRK;
  _crt0_startup_flags |= _CRT0_FLAG_NONMOVE_SBRK;

  auto adapter = DetectGraphicsAdapter();
  e00::GetDefaultLogger().Info(e00::source_location::current(),
                               "Detected adatper: {}", adapter == GraphicsAdapter::VGA ? "VGA" : "EGA");

  if (osInfo.type == DOS::OSType::Type_UNKNOWN) {
    osInfo = DOS::DetectOS();
    e00::GetDefaultLogger().Info(
        e00::source_location::current(),
        "Detected OS: {}", PlatformName());
  }

  __dpmi_version_ret dpmi_version;
  if (__dpmi_get_version(&dpmi_version) != -1) {
    e00::GetDefaultLogger().Info(
        e00::source_location::current(),
        "DPMI version: {}.{}",
        dpmi_version.major, dpmi_version.minor);
  }

  hasNearPtr = __djgpp_nearptr_enable() != 0;
  if (!hasNearPtr) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Unable to enable nearptr");
    return std::make_error_code(std::errc::not_supported);
  }

  e00::GetDefaultLogger().Info(e00::source_location::current(), "Using VGA 12h mode");
  mainSurface = std::make_unique<DOS::VGA_12h>();

  // Make a double buffer
  mainSurfaceBuffer = e00::Bitmap::Create(mainSurface->Size(), mainSurface->GetBitDepth(), mainSurface->GetNumberOfColorsInPalette());

  InitFixedTimer();
  InitKeyboard();
  if (!InitMouse()) {
    // No mouse found ?!
    e00::GetDefaultLogger().Error(e00::source_location::current(), "No mouse found");
  }

  // Force to wait a tick so that time counters aren't all 0
  WaitForNextFixedTick();

  // Dummy error to force an exit
  return {};
}

void Exit() {
  mainSurface.reset();
  QuitMouse();
  QuitKeyboard();
  QuitFixedTimer();
  __djgpp_nearptr_disable();
  exit(0);
}

bool InitEngine(e00::Engine &engine) {
  engine.SetPlatformData(new e00::PlatformData);
  engine.GetPlatformData()->oldTime = std::chrono::microseconds::zero();
  return true;
}

void QuitEngine(e00::Engine &engine) {
  delete engine.GetPlatformData();
}

void ProcessEvents(e00::Engine &engine) {
  WaitForNextFixedTick();

  SendKeyboardEvent(engine);
  SendMouseEvent(engine);

  const auto currentTime = ElapsedFixedTime();
  const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - engine.GetPlatformData()->oldTime);
  engine.GetPlatformData()->oldTime = currentTime;

  engine.Tick(delta);
  e00::ResourceManager::GlobalResourceManager().Tick(delta);
}

void ProcessDraw(e00::Engine &engine) {
  if (mainSurface && mainSurfaceBuffer) {
    if (const auto painter = mainSurfaceBuffer->BeginDraw()) {
      engine.RootWidget()->Paint(*painter);
    }

    if (const auto painter = mainSurface->BeginDraw()) {
      painter->DrawSurface(*mainSurfaceBuffer, {{0, 0}, mainSurfaceBuffer->Size()}, {0, 0});
    }
  }

  engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
}

Surface &GetMainSurface(e00::Engine &engine) {
  return *mainSurface;
}

Surface &GetMainSurface() {
  return *mainSurface;
}

bool HasFocus(e00::Engine &engine) {
  const bool isAWindows = osInfo.type == DOS::OSType::Type_WIN3     // Win 3.x
                          || osInfo.type == DOS::OSType::Type_WIN95 // Win 95-95b
                          || osInfo.type == DOS::OSType::Type_WIN98 // Win 98-98Se
                          || osInfo.type == DOS::OSType::Type_WINME // WinME
                          || osInfo.type == DOS::OSType::Type_WINNT;// WinNT4,2k...

  if (isAWindows) {
    __dpmi_regs regs = {};

    // Check Windows 95 VM Active State via DPMI
    regs.x.ax = 0x1685;// Advanced Power Management / VM status extension
    __dpmi_int(0x2F, &regs);

    // If AX is 0, the feature is supported and details are in BX
    // Bit 0 of BX: 1 = VM is in foreground, 0 = VM is background/suspended
    if (regs.x.ax == 0) {
      return (regs.x.bx & 1) != 0;
    }
  }

  return true;// Fallback for pure DOS (always focused)
}

void Yield() {
  __dpmi_yield();
  asm volatile("" ::: "memory");
}
}// namespace platform
