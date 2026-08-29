#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

namespace win31 {

// Build/attach a native menu bar for the game window.
void ApplyMenu(HWND hwnd, std::span<const platform::MenuItem> items);
void ClearMenu(HWND hwnd);

// Handle WM_COMMAND from the menu; returns true if consumed.
bool HandleMenuCommand(e00::Engine &engine, WPARAM wParam);

} // namespace win31
