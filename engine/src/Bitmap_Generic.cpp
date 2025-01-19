#include <Engine.hpp>

namespace {
uint16_t DepthEnumToBits(e00::Bitmap::BitDepth depth) {
  switch (depth) {
    case e00::Bitmap::BitDepth::DEPTH_1: return 1;
    case e00::Bitmap::BitDepth::DEPTH_8: return 8;
    case e00::Bitmap::BitDepth::DEPTH_32: return 32;
  }
  return 0;
}
}// namespace

namespace e00 {
struct Bitmap::BitmapData {
  uint16_t bytes_per_line;
  size_t alloc_size;
  uint8_t **data;

  BitmapData()
    : bytes_per_line(0),
      alloc_size(0),
      data(nullptr) {
  }

  BitmapData(const BitmapData &other)
    : bytes_per_line(other.bytes_per_line),
      alloc_size(other.alloc_size),
      data(static_cast<uint8_t **>(malloc(alloc_size))) {
    if (!data) {
      return;
    }

    memcpy(static_cast<void *>(data), static_cast<void *>(other.data), alloc_size);
  }

  BitmapData(const Vec2D<uint16_t> &size, BitDepth bit_depth)
    : bytes_per_line(),
      alloc_size(0),
      data(nullptr) {

    auto depth = DepthEnumToBits(bit_depth);

    // Compute image size
    bytes_per_line = ((size.x * depth + 31u) / 32u) * 4u;// bytes per line
    const auto image_size = bytes_per_line * size.y;// image size
    const auto pointer_table = size.y * sizeof(uint8_t *);// pointer table size
    alloc_size = image_size + pointer_table;// total size of data block

    const auto pad = bytes_per_line - (size.x * depth) / 8;// pad per line

    data = static_cast<uint8_t **>(malloc(alloc_size));
    if (!data) {
      return;
    }

    auto height = size.y;
    auto p = data;
    uint8_t *d = reinterpret_cast<uint8_t *>(p) + pointer_table;
    while (height--) {
      *p++ = d;
      if (pad)
        memset(d + bytes_per_line - pad, 0, pad);
      d += bytes_per_line;
    }
  }

  ~BitmapData() {
    free(data);
  }

  uint8_t *get_line(uint16_t y) { return data[y]; }
};

Bitmap::Bitmap(const Vec2D<uint16_t> &size, const BitDepth bit_depth, const int numColorsInPalette)
  : _size(size), _bit_depth(bit_depth) {
  // Allocate palette?
  if (numColorsInPalette > 0) {
    // Validate the number of colors
    if (bit_depth == BitDepth::DEPTH_8 && numColorsInPalette > 256) {
      abort();
    }

    if (bit_depth == BitDepth::DEPTH_1 && numColorsInPalette > 2) {
      abort();
    }

    _palette.resize(numColorsInPalette);
  }

  _data = std::make_unique<BitmapData>(_size, _bit_depth);
}

Bitmap::~Bitmap() {
}


bool Bitmap::HasSamePalette(const Bitmap &other) const {
  return _palette == other._palette;
}

std::unique_ptr<Bitmap> Bitmap::ConvertToDepth(BitDepth bit_depth) const {
  // Fail on no-op
  if (bit_depth == _bit_depth) {
    return nullptr;
  }

  if (bit_depth == BitDepth::DEPTH_32) {
  }
}

std::unique_ptr<Bitmap> Bitmap::ConvertToDepthWithPalette(BitDepth bit_depth, const FixedPalette &palette) const {
  // Fail on no-op
  if (bit_depth == _bit_depth && palette == _palette) {
    return nullptr;
  }

  auto dst = std::make_unique<Bitmap>(_size, bit_depth, palette.size());
  dst->_palette = palette;
  
  if (GetBitDepth() == BitDepth::DEPTH_8) {
    // just a palette swap
    if (dst->GetBitDepth() == BitDepth::DEPTH_8 || dst->GetBitDepth() == BitDepth::DEPTH_1) {
      // We just want to readjust the pixel data to point to the closest match to the new palette
      dst->_data.reset();
      dst->_data = std::make_unique<BitmapData>(*_data);

      // Make srcColorIdx -> dstColorIdx map
      std::array<uint8_t, 256> srcColorIdxToDstColorIdx{};
      for (uint8_t i = 0; i < _palette.size(); i++) {
        srcColorIdxToDstColorIdx[i] = dst->_palette.findClosestColorIndex(_palette[i]);
      }

      // Swap the colors
      auto height = _size.y;
      while (height--) {
        auto dest = dst->_data->get_line(height);
        auto source = _data->get_line(height);
        auto width = _size.x;
        while (width--) {
          dest[width] = srcColorIdxToDstColorIdx[source[width]];
        }
      }
    }

    // 8bit -> 32bit (what?)
    if (dst->GetBitDepth() == BitDepth::DEPTH_32) {
    
    }
  }

  if (GetBitDepth() == BitDepth::DEPTH_32) {
    // 32bit -> 8 bit
    if (dst->GetBitDepth() == BitDepth::DEPTH_8) {
    
    }

    // 32bit -> 1 bit
    if (dst->GetBitDepth() == BitDepth::DEPTH_1) {
    
    }
  }

  return dst;
}

std::unique_ptr<Bitmap> Bitmap::Clone(RectT<uint16_t> copyRect) const {
  copyRect.size = copyRect.size.Clamp(_size);
  copyRect.origin = copyRect.origin.Clamp(_size);

  if (copyRect.To() > _size) {
    return nullptr;
  }

  auto ret = std::make_unique<Bitmap>(copyRect.size, _bit_depth, _palette.size());
  ret->_palette = _palette;
  BitBlit(*ret, {}, *this, copyRect);
  return ret;
}

bool Bitmap::CanDoFastCopyFrom(const Bitmap &src, const RectT<uint16_t> &srcRect, const Vec2D<uint16_t> &dstPos) const noexcept {
  // "Easy" to copy if both have the same depth and one of:
  //   - 32 bit
  //   - 8 bit, identical palette
  //   - 1 bit, identical palette and byte-aligned area
  //
  // Check if the destination and source bitmaps have the same palette
  const auto samePalette = HasSamePalette(src);

  // Check if the depth of the destination bitmap is not equal to 1
  const auto dstNotMonochrome = GetBitDepth() != BitDepth::DEPTH_1;

  // Check if destinationX is aligned to an 8-pixel boundary
  const bool destinationXAligned = (dstPos.x & 7) == 0;

  // Check if sourceX is aligned to an 8-pixel boundary
  const bool sourceXAligned = (srcRect.origin.x & 7) == 0;

  // Check if sourceWidth is a multiple of 8
  const bool sourceWidthAligned = (srcRect.size.x & 7) == 0;

  // Check if the source region is within the bounds of the source bitmap
  const auto sourceWithinBounds = (srcRect.origin.x + srcRect.size.x < src.Size().x);

  // Check if the destination region is within the bounds of the destination bitmap
  const auto destinationWithinBounds = (dstPos.x + srcRect.size.x < Size().x);

  // Combine all alignment checks for depth equal to 1
  const bool alignmentChecks = destinationXAligned && sourceXAligned && (!sourceWidthAligned || (sourceWithinBounds && destinationWithinBounds));

  return samePalette && (dstNotMonochrome || alignmentChecks);
}

void Bitmap::BlitFrom(const Bitmap &src, RectT<uint16_t> srcRect, Vec2D<uint16_t> dstPos) {
  // Adjust the source rectangle to fit within the source bitmap.
  if (srcRect.To() > src.Size()) {
    srcRect.size = src.Size() - srcRect.origin;
  }

  if (dstPos + srcRect.size > Size()) {
    srcRect.size = Size() - dstPos;
  }

  // Early return if there is no area to copy.
  if (srcRect.size.Area() == 0) { return; }

  // Final condition combining all checks
  if (!CanDoFastCopyFrom(src, srcRect, dstPos)) {
    if (GetBitDepth() != BitDepth::DEPTH_32) {
      // Convert the destination to 32bit, blitz to there, then convert it back to
      // dst's original depth and copy it to dst with the palette
      auto dstConv = ConvertToDepth(BitDepth::DEPTH_32);

      // Save memory by delete our data
      _data.reset();

      // Do the original Blitz
      dstConv->BlitFrom(src, srcRect, dstPos);

      // Convert it back to use our palette
      const auto dstConvBack = dstConv->ConvertToDepthWithPalette(GetBitDepth(), _palette);

      // Do not keep the memory of the original conversion
      dstConv.reset();

      // Use the converted bitmap's data as our own
      _data.swap(dstConvBack->_data);

      return;
    }
  }

  // Do we need to do depth conversion?
  if (GetBitDepth() != src.GetBitDepth()) {
    if (srcRect.size == src.Size() || GetBitDepth() == BitDepth::DEPTH_32) {
      const auto srcConverted = src.ConvertToDepth(GetBitDepth());
      BlitFrom(*srcConverted, srcRect, dstPos);
    } else {
      const auto srcConverted = src.Clone(srcRect);
      BlitFrom(*srcConverted, { { 0, 0 }, srcConverted->Size() }, dstPos);
    }
    return;
  }

  // Now assume both are 32-bit or 8-bit with compatible palettes or 1-bit with aligned data.

  uint8_t *dstAddr = _data->get_line(dstPos.y);
  uint8_t *srcAddr = src._data->get_line(srcRect.origin.y);
  uint16_t dstAddPerLine = _data->bytes_per_line;
  uint16_t srcAddPerLine = src._data->bytes_per_line;
  size_t copySize = 0;

  switch (src.GetBitDepth()) {
    case BitDepth::DEPTH_1:
      dstAddr += dstPos.x / 8;
      srcAddr += srcRect.origin.x / 8;
      copySize = (srcRect.size.x + 7) / 8;
      break;

    case BitDepth::DEPTH_8:
      dstAddr += dstPos.x;
      srcAddr += srcRect.origin.x;
      copySize = srcRect.size.x;
      break;

    case BitDepth::DEPTH_32:
      dstAddr += dstPos.x;
      srcAddr += srcRect.origin.x;
      dstAddPerLine = Size().x;
      srcAddPerLine = src.Size().x;
      copySize = srcRect.size.x * sizeof(Color);
      break;
  }

  auto height = srcRect.size.y;
  while (height--) {
    memcpy(dstAddr, srcAddr, copySize);
    dstAddr += dstAddPerLine;
    srcAddr += srcAddPerLine;
  }
}

void BitBlit(Bitmap &dst, Vec2D<uint16_t> dstPos, const Bitmap &src, RectT<uint16_t> srcRect) {
  dst.BlitFrom(src, srcRect, dstPos);
}


}// namespace e00
