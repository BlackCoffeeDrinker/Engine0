
#include "MetalSurface.h"
#include "../Painter_PaintDevice.hpp"

namespace apple {
MetalSurface::MetalSurface(const e00::Vec2D<uint16_t> &size,
                 e00::DrawableSurface::BitDepth depth,
                 uint16_t scale)
  : _scale(std::max<uint16_t>(scale, 1)),
    _bitDepth(depth),
    _size(size),
    _data(size, depth) {
}

void MetalSurface::SetScale(uint16_t scale) {
    const auto newScale = std::max<uint16_t>(scale, 1);
    if (_scale == newScale) {
      return;
    }

    _scale = newScale;
    _presentTexture = nil;
}

MTLTextureDescriptor* MetalSurface::getDescriptor() const {
    const auto size = PresentedSize();
    MTLTextureDescriptor *descriptor =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                           width:size.x
                                                          height:size.y
                                                       mipmapped:NO];

    descriptor.usage = MTLTextureUsageShaderRead;
    descriptor.storageMode = MTLStorageModeShared;
    
    return descriptor;
}

bool MetalSurface::SupportsOptimizedCopyFrom(const DrawableSurface &source) const {
    return source.Type() == Type();
}

std::unique_ptr<e00::Painter> MetalSurface::BeginDraw() {
    return std::make_unique<e00::SoftwarePainter>(_size, _bitDepth, _palette, _data);
}

std::unique_ptr<e00::DrawableSurface> MetalSurface::CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) {
    return std::make_unique<MetalSurface>(e00::Vec2D<uint16_t>{static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y)}, _bitDepth, _scale);
}

void MetalSurface::ReadLineInto(
    e00::BitmapSizeType line,
    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
    const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const {
    _data.ReadLineInto(line, startX, endX, targetInformation, _bitDepth, _palette, targetBuffer);
}

bool MetalSurface::UploadToTexture(id<MTLDevice> device) {
    if (!_presentTexture) {
      return false;
    }
    
    const auto size = Size();
    const NSUInteger bytesPerRow = size.x * 4;
    const MTLRegion region = {
        {0, 0, 0},
        {size.x, size.y, 1}};
    
    _presentPixels.resize(static_cast<size_t>(size.x) * static_cast<size_t>(size.y) * 4);
    
    using enum e00::DrawableSurface::BitDepth;
    switch (GetBitDepth()) {
      case DEPTH_32: {
        for (e00::BitmapSizeType y = 0; y < size.y; ++y) {
          const auto srcLine = _data.GetLineSpan(y);
          auto *dstLine = _presentPixels.data() + static_cast<size_t>(y) * size.x * 4;

          std::memcpy(dstLine, srcLine.data(), static_cast<size_t>(size.x) * 4);
        }
        break;
      }

      case DEPTH_8: {
        const auto paletteSize = GetNumberOfColorsInPalette();

        for (e00::BitmapSizeType y = 0; y < size.y; ++y) {
          const auto srcLine = _data.GetLineSpan(y);
          auto *dstLine = _presentPixels.data() + static_cast<size_t>(y) * size.x * 4;

          for (e00::BitmapSizeType x = 0; x < size.x; ++x) {
            const auto paletteIndex = e00::helpers::BitmapDepth8::ReadColor(srcLine, x);
            const auto color = paletteIndex < paletteSize
                                   ? GetColorFromPalette(paletteIndex)
                                   : e00::Color{};

            dstLine[x * 4 + 0] = color.blue;
            dstLine[x * 4 + 1] = color.green;
            dstLine[x * 4 + 2] = color.red;
            dstLine[x * 4 + 3] = 0xFF;
          }
        }
        break;
      }

      case DEPTH_1: {
        const auto paletteSize = GetNumberOfColorsInPalette();
        const auto color0 = paletteSize > 0 ? GetColorFromPalette(0) : e00::Color{0, 0, 0};
        const auto color1 = paletteSize > 1 ? GetColorFromPalette(1) : e00::Color{255, 255, 255};

        for (e00::BitmapSizeType y = 0; y < size.y; ++y) {
          const auto srcLine = _data.GetLineSpan(y);
          auto *dstLine = _presentPixels.data() + static_cast<size_t>(y) * size.x * 4;

          for (e00::BitmapSizeType x = 0; x < size.x; ++x) {
            const auto color = e00::helpers::BitmapDepth1::ReadColor(srcLine, x) ? color1 : color0;

            dstLine[x * 4 + 0] = color.blue;
            dstLine[x * 4 + 1] = color.green;
            dstLine[x * 4 + 2] = color.red;
            dstLine[x * 4 + 3] = 0xFF;
          }
        }
        break;
      }

      case DEPTH_INVALID:
      default:
        std::fill(_presentPixels.begin(), _presentPixels.end(), uint8_t{0});
        break;
    }
    
    [_presentTexture replaceRegion:region
                        mipmapLevel:0
                          withBytes:_presentPixels.data()
                        bytesPerRow:bytesPerRow];

    return true;
  }

}// namespace apple
