
#include "Bitmap.hpp"

namespace {

SDL_PixelFormat BitDepthToPixelFormat(const e00::Bitmap::BitDepth depth) {
  switch (depth) {
    case e00::Bitmap::BitDepth::DEPTH_1:
    case e00::Bitmap::BitDepth::DEPTH_8:
      return SDL_PIXELFORMAT_INDEX8;
    case e00::Bitmap::BitDepth::DEPTH_32:
      return SDL_PIXELFORMAT_ARGB8888;
  }
}

int BitDepthToColorPaletteSize(const e00::Bitmap::BitDepth depth) {
  switch (depth) {
    case e00::Bitmap::BitDepth::DEPTH_1:
      return 2;
    case e00::Bitmap::BitDepth::DEPTH_8:
      return 256;
    case e00::Bitmap::BitDepth::DEPTH_32:
      return 0;
  }
}
}// namespace

namespace platform {
SDLBitmap::SDLBitmap(SDL_Surface *fromSurface)
    : _surface(fromSurface) {
}

SDLBitmap::SDLBitmap(int w, int h, e00::Bitmap::BitDepth bpp)
    : _surface(SDL_CreateSurface(w, h, BitDepthToPixelFormat(bpp))),
      _palette(bpp != BitDepth::DEPTH_32 ? SDL_CreatePalette(BitDepthToColorPaletteSize(bpp)) : nullptr) {
  if (_palette) {
    SDL_SetSurfacePalette(_surface, _palette);
  }
}

SDLBitmap::~SDLBitmap() {
  SDL_DestroySurface(_surface);
  if (_palette) {
    SDL_DestroyPalette(_palette);
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
  const auto format = SDL_GetPixelFormatDetails(_surface->format);
  const auto bpp = format->bytes_per_pixel;

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

  const auto format = SDL_GetPixelFormatDetails(_surface->format);
  SDL_LockSurface(_surface);
  for (auto height = 0; height < Size().y; ++height) {
    auto srcLineData = bitmap.GetLineData(height);
    Uint8 *startPixels = static_cast<Uint8 *>(_surface->pixels) + height * _surface->pitch;

    memcpy(startPixels, srcLineData.data(), Size().x * format->bytes_per_pixel);
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

  const auto format = SDL_GetPixelFormatDetails(_surface->format);
  SDL_LockSurface(_surface);
  for (auto height = 0; height < copyRect.size.y; ++height) {
    auto *startPixels = static_cast<Uint8 *>(_surface->pixels) + (height + copyRect.origin.y) * _surface->pitch;
    auto dst = bmp->GetLineData(height);
    memcpy(dst.data(), startPixels + copyRect.origin.x * format->bytes_per_pixel, copyRect.size.x * format->bytes_per_pixel);
  }

  SDL_UnlockSurface(_surface);
  return bmp;
}

void SDLBitmap::ReadLineInto(
    e00::BitmapSizeType line,
    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
    const e00::DrawableSurface::TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const {

  const auto format = SDL_GetPixelFormatDetails(_surface->format);
  const auto bpp = format->bytes_per_pixel;
  const auto width = endX - startX;

  SDL_LockSurface(_surface);
  Uint8 *srcPixels = static_cast<Uint8 *>(_surface->pixels) + line * _surface->pitch + startX * bpp;

  // For now, let's assume a simple copy if depths match and it's 8-bit or 32-bit standard
  // This should be improved to use targetInformation
  if (GetBitDepth() == targetInformation.bit_depth) {
    memcpy(targetBuffer.data(), srcPixels, width * bpp);
  } else {
    // Fallback: convert through e00::Bitmap (slow but works)
    // TODO: implement direct conversion
    auto bmp = ConvertToBitmap();
    bmp->ReadLineInto(line, startX, endX, targetInformation, targetBuffer);
  }

  SDL_UnlockSurface(_surface);
}

std::unique_ptr<e00::Painter> SDLBitmap::BeginDraw() {
  // SDL3 doesn't have a direct software painter that we use, 
  // so we'll wrap it in a way that SoftwarePainter can work on it if we provide GetLineData,
  // or we just return a SoftwarePainter after converting/locking.
  // Actually, let's just use SoftwarePainter for now.
  // We need to make sure SoftwarePainter can work on SDLBitmap.
  return std::make_unique<e00::SoftwarePainter>(*this);
}

std::unique_ptr<e00::DrawableSurface> SDLBitmap::CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) {
  return std::make_unique<SDLBitmap>(static_cast<int>(size.x), static_cast<int>(size.y), GetBitDepth());
}

void SDLBitmap::BlitFrom(const DrawableSurface &src, e00::RectT<uint16_t> srcRect, e00::Vec2D<uint16_t> dstPos) {
  SDL_LockSurface(_surface);
  // ...
  SDL_UnlockSurface(_surface);
}

std::span<uint8_t const> SDLBitmap::GetLineData(e00::BitmapSizeType y) const {
  return std::span<uint8_t const>(static_cast<uint8_t *>(_surface->pixels) + y * _surface->pitch, _surface->pitch);
}

std::span<uint8_t> SDLBitmap::GetLineData(e00::BitmapSizeType y) {
  return std::span<uint8_t>(static_cast<uint8_t *>(_surface->pixels) + y * _surface->pitch, _surface->pitch);
}


std::error_code SDLBitmap::SaveToBMP(e00::WritableStream &writableStream) const {
  SDL_IOStreamInterface ops{};

  ops.close = [](void *userdata) -> bool { return true; };

  ops.read = [](void *userdata, void *ptr, size_t size, SDL_IOStatus *status) -> size_t {
    const auto stream = static_cast<e00::Stream *>(userdata);
    if (auto ret = stream->Read(size, ptr)) {
      *status = SDL_IO_STATUS_ERROR;
      return 0;
    }

    *status = SDL_IO_STATUS_READY;
    return size;
  };

  ops.write = [](void *userdata, const void *ptr, size_t size, SDL_IOStatus *status) -> size_t {
    const auto stream = static_cast<e00::WritableStream *>(userdata);
    if (auto ret = stream->Write(size, ptr)) {
      *status = SDL_IO_STATUS_ERROR;
      return 0;
    }

    *status = SDL_IO_STATUS_READY;
    return size;
  };

  ops.size = [](void *userdata) -> Sint64 {
    auto stream = static_cast<e00::Stream *>(userdata);
    return static_cast<Sint64>(stream->AvailableToRead());
  };

  ops.seek = [](void *userdata, Sint64 offset, SDL_IOWhence whence) -> Sint64 {
    auto stream = static_cast<e00::Stream *>(userdata);

    if (whence == SDL_IO_SEEK_SET) {
      return stream->SeekTo(offset) ? -1 : static_cast<Sint64>(stream->Position());
    }

    if (whence == SDL_IO_SEEK_CUR) {
      return stream->SeekTo(offset + stream->Position()) ? -1 : static_cast<Sint64>(stream->Position());
    }

    if (whence == SDL_IO_SEEK_END) {
      return stream->SeekTo(stream->Size() + offset) ? -1 : static_cast<Sint64>(stream->Position());
    }

    return -1;
  };

  auto io = SDL_OpenIO(&ops, &writableStream);
  SDL_SaveBMP_IO(_surface, io, false);
  SDL_CloseIO(io);
  return {};
}

std::unique_ptr<Surface> CreateSurface(const e00::Vec2D<uint16_t> &size, e00::DrawableSurface::BitDepth depth) {
  return std::make_unique<SDLBitmap>(size.x, size.y, depth);
}

}// namespace platform
