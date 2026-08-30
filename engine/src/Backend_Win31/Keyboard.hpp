#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

namespace win31 {

void InitKeyboard();
void QuitKeyboard();

// Translate a WM_KEY* message into an engine InputEvent and dispatch it.
void HandleKeyboardMessage(e00::Engine &engine, UINT msg, WPARAM wParam, LPARAM lParam);

}// namespace win31
