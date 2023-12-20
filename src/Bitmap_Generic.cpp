#include <Engine.hpp>
#include "Engine/Resource/Bitmap.hpp"


namespace e00 {

std::error_code Bitmap::Blit(const e00::Bitmap &source, const RectT<uint16_t> &source_rect, const Vec2D<uint16_t> &destination_position) {

  // Are we blitting between the same BPP ?
  if (source.GetBitDepth() != GetBitDepth()) {
    // TODO: Convert to destination BPP then continue
    return {};
  }

  // Handle copying to self
  if (this == &source) {
    // TODO: Blit to self
    return {};
  }

  // Are we copying the same type?
  if (GetType() == source.GetType()) {
    if (GetType() == Type::SOFTWARE) {
      // Software-to-Software
    } else {
      // Platform-to-Platform
    }
  } else {

  }


  // Unreachable
  return {};
}

SoftwareBitmap *Bitmap::GetSoftwareBitmap() {
  if (GetType() == Type::SOFTWARE)
    return static_cast<SoftwareBitmap *>(this);

  return nullptr;
}

SoftwareBitmap::SoftwareBitmap(const Vec2D<uint16_t> &size, Bitmap::BitDepth depth)
  : _size(size),
    _bpp(depth),
    _bitmapData(static_cast<size_t>(PixelStride()) * _size.Area()) {

}

SoftwareBitmap::~SoftwareBitmap() {
}



}// namespace e00
