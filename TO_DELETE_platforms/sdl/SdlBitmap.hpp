
#ifndef ENGINE00_SDLBITMAP_HPP
#define ENGINE00_SDLBITMAP_HPP

#include "Platform.h"
#include <SDL3/SDL.h>

namespace platform {
class SDLBitmap : public Surface {
  SDL_Surface *_surface;
  SDL_Palette *_palette{};

public:
  NOT_COPYABLE(SDLBitmap);

  explicit SDLBitmap(SDL_Surface *fromSurface);
  SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp);
  ~SDLBitmap() override;


  [[nodiscard]] BitDepth GetBitDepth() const override {
    if (_surface && _surface->format) {
      return _surface->format->palette ? BitDepth::DEPTH_8 : BitDepth::DEPTH_32;
    }

    return BitDepth::DEPTH_32;
  }

  [[nodiscard]] e00::Vec2D<e00::BitmapSizeType> Size() const override {
    if (_surface) {
      return {
          static_cast<e00::BitmapSizeType>(_surface->w),
          static_cast<e00::BitmapSizeType>(_surface->h)};
    }

    return {};
  }

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<SDLBitmap>(); }

  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override;
  [[nodiscard]] e00::Color GetColorFromPalette(size_t index) const override;

  [[nodiscard]] std::unique_ptr<e00::Bitmap> ConvertToBitmap() const override;
  [[nodiscard]] std::error_code ConvertFromBitmap(const e00::Bitmap &bitmap) override;
  [[nodiscard]] std::unique_ptr<e00::Bitmap> Clone(e00::RectT<e00::BitmapSizeType> copyRect) const override;
  void BlitFrom(const DrawableSurface &src, e00::RectT<e00::BitmapSizeType> srcRect, e00::Vec2D<e00::BitmapSizeType> dstPos) override;
  [[nodiscard]] std::error_code SaveToBMP(e00::WritableStream &writableStream) const override;
};

}// namespace platform


#endif//ENGINE00_SDLBITMAP_HPP
