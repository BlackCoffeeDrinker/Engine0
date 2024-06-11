
#ifndef ENGINE00_SDLBITMAP_HPP
#define ENGINE00_SDLBITMAP_HPP

#include <Platform.hpp>
#include <SDL.h>

namespace e00::platform {
class SDLBitmap : public e00::Bitmap {
  SDL_Surface *_surface;

public:
  NOT_COPYABLE(SDLBitmap);

  explicit SDLBitmap(SDL_Surface *fromSurface) : _surface(fromSurface) {
  }

  SDLBitmap(int w, int h);
  SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp);
  ~SDLBitmap() override;

  Type GetType() const override {
    return Type::SOFTWARE;
  }


  BitDepth GetBitDepth() const override {
    return BitDepth::TRUE_COLOR_888;
  }

  [[nodiscard]] auto Surface() const { return _surface; }

  [[nodiscard]] e00::Vec2D<uint16_t> Size() const override {
    if (_surface) {
      return {
          static_cast<uint16_t>(_surface->w),
          static_cast<uint16_t>(_surface->h)};
    }

    return {};
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override;

  void SetPixel(const e00::Vec2D<uint16_t> &position, const e00::Color &color) override;
};

}// namespace e00::platform


#endif//ENGINE00_SDLBITMAP_HPP
