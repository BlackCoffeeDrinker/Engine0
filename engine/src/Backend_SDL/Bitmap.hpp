
#ifndef ENGINE00_SDLBITMAP_HPP
#define ENGINE00_SDLBITMAP_HPP

#include "Platform.hpp"
#include <SDL3/SDL.h>

namespace platform {
class SDLBitmap : public Surface {
  SDL_Surface *_surface;
  SDL_Palette *_palette{};

public:
  explicit SDLBitmap(SDL_Surface *fromSurface);
  SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp);
  ~SDLBitmap() override;


  [[nodiscard]] e00::DrawableSurface::BitDepth GetBitDepth() const override {
    const auto *palette = SDL_GetSurfacePalette(_surface);
    if (!palette) {
      return e00::DrawableSurface::BitDepth::DEPTH_32;
    }

    return e00::DrawableSurface::BitDepth::DEPTH_8;
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

  [[nodiscard]] std::unique_ptr<e00::Painter> BeginDraw() override;

  [[nodiscard]] std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) override;

  void ReadLineInto(
      e00::BitmapSizeType line,
      e00::BitmapSizeType startX, e00::BitmapSizeType endX,
      const e00::DrawableSurface::TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const override;

  void BlitFrom(const DrawableSurface &src, e00::RectT<e00::BitmapSizeType> srcRect, e00::Vec2D<e00::BitmapSizeType> dstPos) override;

  [[nodiscard]] std::span<uint8_t const> GetLineData(e00::BitmapSizeType y) const override;
  [[nodiscard]] std::span<uint8_t> GetLineData(e00::BitmapSizeType y) override;

  [[nodiscard]] std::error_code SaveToBMP(e00::WritableStream &writableStream) const;
};

}// namespace platform


#endif//ENGINE00_SDLBITMAP_HPP
