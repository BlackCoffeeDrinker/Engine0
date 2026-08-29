#include "Platform.hpp"
#include "Win32Types.hpp"

// Provided by the game executable (see example/src/main.cpp). Returns process exit code.
extern "C" int e00_app_main();

namespace win31 {
void SetAppInstance(HINSTANCE inst);
}

// Freestanding Win32s process entry. Selected via toolchain linker flag:
//   -Wl,-e,_WinMainCRTStartup@0
// (stdcall name decoration on i386). No CRT startup — heap is Heap.cpp,
// memcpy/etc. are Intrinsics.cpp.
extern "C" void WINAPI WinMainCRTStartup() {
  e00::GetDefaultLogger().Info(e00::source_location::current(), "--------------------------------------------");
  e00::GetDefaultLogger().Info(e00::source_location::current(), "Win32s compatible exec started !");
  e00::GetDefaultLogger().Info(e00::source_location::current(), "--------------------------------------------");

  win31::SetAppInstance(GetModuleHandleA(nullptr));
  const int code = e00_app_main();
  ExitProcess(static_cast<UINT>(code < 0 ? 1 : code));
}
