#include "Platform.hpp"
#include "Win32Types.hpp"

// Provided by the game executable (see example/src/main.cpp). Returns process exit code.
extern "C" int e00_app_main();

// Defined in Intrinsics.cpp: walks the linker-provided "__CTOR_LIST__" to run
// every namespace-scope C++ object's constructor (see the comment there for
// why this is otherwise skipped entirely under -nostartfiles/-nodefaultlibs).
extern "C" void RunGlobalConstructors();

// Defined in Intrinsics.cpp: runs everything registered via __cxa_atexit
// (i.e. global object destructors), which only actually get registered once
// RunGlobalConstructors() above has run.
extern "C" void __cxa_finalize(void *dso);

namespace win31 {
void SetAppInstance(HINSTANCE inst);
}

// Freestanding Win32s process entry. Selected via toolchain linker flag:
//   -Wl,-e,_WinMainCRTStartup@0
// (stdcall name decoration on i386). No CRT startup — heap is Heap.cpp,
// memcpy/etc. are Intrinsics.cpp.
extern "C" void WINAPI WinMainCRTStartup() {
  // Must run before anything else (including the very first logger call
  // below): the logger, and effectively every other engine subsystem, may
  // transitively rely on namespace-scope C++ objects (e.g. Font.cpp's
  // ascii_data/ascii_default glyph tables) having actually been constructed.
  RunGlobalConstructors();

  e00::GetDefaultLogger().Info(e00::source_location::current(), "--------------------------------------------");
  e00::GetDefaultLogger().Info(e00::source_location::current(), "Win32s compatible exec started !");
  e00::GetDefaultLogger().Info(e00::source_location::current(), "--------------------------------------------");

  win31::SetAppInstance(GetModuleHandleA(nullptr));
  const int code = e00_app_main();
  __cxa_finalize(nullptr);
  ExitProcess(static_cast<UINT>(code < 0 ? 1 : code));
}
