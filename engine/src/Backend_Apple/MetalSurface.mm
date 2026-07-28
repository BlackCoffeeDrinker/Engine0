
#include "MetalSurface.h"
#include "MetalSurfacePainter.hpp"
#include "../Painter_PaintDevice.hpp"

namespace apple {
MetalSurface::MetalSurface(const e00::Vec2D<uint16_t> &size,
                 e00::DrawableSurface::BitDepth depth,
                 uint16_t scale)
  : _size(size),
    _scale(std::max<uint16_t>(scale, 1)),
    _presentPixels(static_cast<size_t>(_size.x) * static_cast<size_t>(_size.y) * 4),
    _palette(0xFF),
    _bitDepth(depth) {
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
    return std::make_unique<MetalSurfacePainter>(_palette, _presentPixels, Size().x * 4, Size().x * 4);
}

std::unique_ptr<e00::DrawableSurface> MetalSurface::CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) {
    return std::make_unique<MetalSurface>(e00::Vec2D<uint16_t>{static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y)}, GetBitDepth(), _scale);
}

void MetalSurface::ReadLineInto(
    e00::BitmapSizeType line,
    e00::BitmapSizeType startX, e00::BitmapSizeType endX,
    const TargetInformation &targetInformation, const std::span<uint8_t>& targetBuffer) const {
   std::abort();
}

void MetalSurface::ReadTransparencyMaskLineInto(e00::BitmapSizeType line, e00::BitmapSizeType startX, e00::BitmapSizeType endX, const std::span<uint8_t> &targetBuffer) const {
    std::abort();
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
       
    [_presentTexture replaceRegion:region
                        mipmapLevel:0
                          withBytes:_presentPixels.data()
                        bytesPerRow:bytesPerRow];

    return true;
  }

}// namespace apple
