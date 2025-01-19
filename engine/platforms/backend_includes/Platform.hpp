
#pragma once

#include <Engine.hpp>

namespace platform {
std::string_view PlatformName();

void SetSettings(std::string_view key,
                 std::string_view value);

std::error_code Init();
void Exit();

void Yield();
void SetWindowTitle(const std::string_view &windowTitle);
bool HasFocus();
void ProcessEvents(e00::Engine &engine);
void ProcessDraw(e00::Engine &engine);

// Open a raw file on disk
std::unique_ptr<e00::Stream> OpenStream(const std::string &name);
}// namespace platform
