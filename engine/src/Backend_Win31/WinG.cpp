#include "WinG.hpp"

#include "Platform.hpp"

namespace win31 {
namespace {

WinGApi g_wing{};
bool g_attempted = false;

template<typename T>
T LoadProc(HMODULE mod, const char *name) {
  return reinterpret_cast<T>(GetProcAddress(mod, name));  
}

}// namespace

bool EnsureWinGLoaded() {
  if (g_attempted) {
    return g_wing.Available();
  }
  g_attempted = true;

  HMODULE mod = LoadLibraryA("WING32.DLL");
  if (!mod) {
    mod = LoadLibraryA("WING.DLL");
  }
  if (!mod) {
    MessageBoxA(nullptr, "WinG not found (WING32.DLL/WING.DLL); falling back to plain GDI DIB blits", "Hello", 0x00000000L);

    e00::GetDefaultLogger().Warning(
        e00::source_location::current(),
        "WinG not found (WING32.DLL/WING.DLL); falling back to plain GDI DIB blits");
    return false;
  }

  g_wing.module = mod;
  g_wing.RecommendDIBFormat = LoadProc<PFN_WinGRecommendDIBFormat>(mod, "WinGRecommendDIBFormat");
  g_wing.CreateDC = LoadProc<PFN_WinGCreateDC>(mod, "WinGCreateDC");
  g_wing.CreateBitmap = LoadProc<PFN_WinGCreateBitmap>(mod, "WinGCreateBitmap");
  g_wing.BitBlt = LoadProc<PFN_WinGBitBlt>(mod, "WinGBitBlt");
  g_wing.StretchBlt = LoadProc<PFN_WinGStretchBlt>(mod, "WinGStretchBlt");
  g_wing.CreateHalftonePalette = LoadProc<PFN_WinGCreateHalftonePalette>(mod, "WinGCreateHalftonePalette");

  if (!g_wing.Available()) {
    MessageBoxA(nullptr, "WinG loaded but required exports missing; falling back to plain GDI", "Hello", 0x00000000L);

    e00::GetDefaultLogger().Warning(
        e00::source_location::current(),
        "WinG loaded but required exports missing; falling back to plain GDI");
    UnloadWinG();
    return false;
  }

  e00::GetDefaultLogger().Info(e00::source_location::current(), "WinG loaded successfully");
  return true;
}

const WinGApi &GetWinG() {
  EnsureWinGLoaded();
  return g_wing;
}

void UnloadWinG() {
  if (g_wing.module) {
    FreeLibrary(g_wing.module);
  }
  g_wing = WinGApi{};
  // Keep g_attempted true so we don't spam reload failures every frame.
}

}// namespace win31
