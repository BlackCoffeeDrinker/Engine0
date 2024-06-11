
#pragma once

#include <Engine.hpp>

namespace platform {
std::string_view PlatformName();

std::error_code Init();
void Exit();

void Yield();
bool HasFocus();
void ProcessEvents(e00::Engine &engine);
void ProcessDraw(e00::Engine &engine);

std::unique_ptr<e00::Bitmap> CreatePlatformBitmap(e00::Vec2D<uint16_t> size, e00::Bitmap::BitDepth bpp);
std::unique_ptr<e00::Stream> OpenStream(const std::string &name);
}// namespace platform
