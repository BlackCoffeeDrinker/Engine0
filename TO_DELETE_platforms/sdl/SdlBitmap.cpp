
#include "SdlBitmap.hpp"

namespace {
int BitDepthEnumToBits(const e00::Bitmap::BitDepth depth) {
  switch (depth) {
    case e00::DrawableSurface::BitDepth::DEPTH_1:
      return 1;
    case e00::DrawableSurface::BitDepth::DEPTH_8:
      return 8;
    case e00::DrawableSurface::BitDepth::DEPTH_32:
      return 32;
  }
  return 0;
}
}// namespace

namespace platform {
SDLBitmap::SDLBitmap(SDL_Surface *fromSurface)
    : _surface(fromSurface) {
}

SDLBitmap::SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp)
    : _surface(SDL_CreateRGBSurface(0, w, h, BitDepthEnumToBits(bpp), 0, 0, 0, 0)),
      _palette(bpp != BitDepth::DEPTH_32 ? SDL_AllocPalette(255) : nullptr) {
  if (_palette) {
    SDL_SetSurfacePalette(_surface, _palette);
  }
}

SDLBitmap::~SDLBitmap() {
  SDL_FreeSurface(_surface);
  if (_palette) {
    SDL_FreePalette(_palette);
  }
}

e00::Color SDLBitmap::GetColorFromPalette(size_t index) const {
  if (!_palette) {
    return {};
  }

  return {_palette->colors[index].r, _palette->colors[index].g, _palette->colors[index].b};
}

size_t SDLBitmap::GetNumberOfColorsInPalette() const {
  if (!_palette) {
    return 0;
  }

  return _palette->ncolors;
}

std::unique_ptr<e00::Bitmap> SDLBitmap::ConvertToBitmap() const {
  const auto bpp = _surface->format->BytesPerPixel;

  auto bmp = std::make_unique<e00::Bitmap>(Size(), GetBitDepth(), 0);
  SDL_LockSurface(_surface);
  for (auto height = 0; height < Size().y; ++height) {
    Uint8 *startPixels = static_cast<Uint8 *>(_surface->pixels) + height * _surface->pitch;
    auto dstLineData = bmp->GetLineData(height);

    memcpy(dstLineData.data(), startPixels, Size().x * bpp);
  }
  SDL_UnlockSurface(_surface);
  return bmp;
}

std::error_code SDLBitmap::ConvertFromBitmap(const e00::Bitmap &bitmap) {
  if (bitmap.GetBitDepth() != GetBitDepth()) {
    const auto tmpBitmap = bitmap.ConvertToDepth(GetBitDepth());
    return ConvertFromBitmap(*tmpBitmap);
  }

  SDL_LockSurface(_surface);
  for (auto height = 0; height < Size().y; ++height) {
    auto srcLineData = bitmap.GetLineData(height);
    Uint8 *startPixels = static_cast<Uint8 *>(_surface->pixels) + height * _surface->pitch;

    memcpy(startPixels, srcLineData.data(), Size().x * _surface->format->BytesPerPixel);
  }
  SDL_UnlockSurface(_surface);

  return {};
}

std::unique_ptr<e00::Bitmap> SDLBitmap::Clone(e00::RectT<uint16_t> copyRect) const {
  auto bmp = std::make_unique<e00::Bitmap>(copyRect.size, GetBitDepth(), GetNumberOfColorsInPalette());
  // Copy palette
  for (auto i = 0; i < GetNumberOfColorsInPalette(); ++i) {
    if (auto ec = bmp->SetPaletteColor(i, GetColorFromPalette(i))) {
      return nullptr;
    }
  }

  SDL_LockSurface(_surface);

  for (auto height = 0; height < copyRect.size.y; ++height) {
    auto *startPixels = static_cast<Uint8 *>(_surface->pixels) + (height + copyRect.origin.y) * _surface->pitch;
    auto dst = bmp->GetLineData(height);
    memcpy(dst.data(), startPixels + copyRect.origin.x * _surface->format->BytesPerPixel, copyRect.size.x * _surface->format->BytesPerPixel);
  }

  SDL_UnlockSurface(_surface);
  return bmp;
}

void SDLBitmap::BlitFrom(const DrawableSurface &src, e00::RectT<uint16_t> srcRect, e00::Vec2D<uint16_t> dstPos) {
  SDL_LockSurface(_surface);

  SDL_UnlockSurface(_surface);
}

std::error_code SDLBitmap::SaveToBMP(e00::WritableStream &writableStream) const {
  SDL_RWops ops;
  ops.hidden.unknown.data1 = &writableStream;
  ops.type = SDL_RWOPS_UNKNOWN;

  ops.close = [](SDL_RWops *) -> int { return 0; };
  ops.read = [](SDL_RWops *context, void *ptr, size_t size, size_t maxnum) -> size_t { return static_cast<e00::Stream *>(context->hidden.unknown.data1)->Read(size * maxnum, ptr) ? 0 : maxnum; };
  ops.write = [](SDL_RWops *context, const void *ptr, size_t size, size_t num) -> size_t { return static_cast<e00::WritableStream *>(context->hidden.unknown.data1)->Write(size * num, ptr) ? 0 : num; };
  ops.size = [](SDL_RWops *context) -> Sint64 { return static_cast<e00::Stream *>(context->hidden.unknown.data1)->AvailableToRead(); };
  ops.seek = [](SDL_RWops *context, Sint64 offset, int whence) -> Sint64 {
    auto stream = static_cast<e00::Stream *>(context->hidden.unknown.data1);
    if (whence == RW_SEEK_SET) {
      return stream->SeekTo(offset) ? -1 : static_cast<Sint64>(stream->Position());
    }

    if (whence == RW_SEEK_CUR) {
      return stream->SeekTo(offset + stream->Position()) ? -1 : static_cast<Sint64>(stream->Position());
    }

    return -1;
  };

  SDL_SaveBMP_RW(_surface, &ops, 0);
  return {};
}

std::unique_ptr<Surface> CreateSurface(const e00::Vec2D<uint16_t> &size, e00::DrawableSurface::BitDepth depth) {
  return std::make_unique<SDLBitmap>(size.x, size.y, depth);
}

}// namespace platform
