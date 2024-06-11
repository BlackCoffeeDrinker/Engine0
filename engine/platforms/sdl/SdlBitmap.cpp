
#include "SdlBitmap.hpp"

namespace e00::platform {

SDLBitmap::SDLBitmap(int w, int h)
    : _surface(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0)) {
}

SDLBitmap::SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp)
    : _surface(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0)) {
}

SDLBitmap::~SDLBitmap() {
  SDL_FreeSurface(_surface);
}

e00::Color SDLBitmap::GetPixel(const Vec2D<uint16_t> &position) {
  return {};
}

void SDLBitmap::SetPixel(const e00::Vec2D<uint16_t> &position, const e00::Color &color) {
}

}// namespace e00::platform
