#pragma once

#include "Platform.hpp"
#include "Win32Types.hpp"

namespace win31 {

void InitMouse();
void QuitMouse();

void HandleMouseMessage(e00::Engine &engine, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PollMouse(e00::Engine &engine, HWND hwnd);

}// namespace win31
