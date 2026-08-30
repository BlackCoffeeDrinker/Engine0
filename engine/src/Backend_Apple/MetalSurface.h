#pragma once

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../Platform.hpp"
#include "ApplePrivateInclude.h"

namespace apple {

class MetalSurface final : public platform::Surface {
  const e00::Vec2D<uint16_t> _size;
  uint16_t _scale = 1;
  bool paletteChanged = false;

  id<MTLTexture> _presentTexture = nil;
  std::vector<uint8_t> _presentPixels;
  e00::FixedPalette _palette;
  BitDepth _bitDepth;

public:
  MetalSurface(const e00::Vec2D<uint16_t> &size,
               BitDepth depth,
               uint16_t scale);

  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<MetalSurface>(); }

  void SetScale(uint16_t scale);


  void DiscardPalette() override { /* No-op */ }

  [[nodiscard]] uint16_t Scale() const noexcept { return _scale; }
  [[nodiscard]] e00::Vec2D<e00::BitmapSizeType> PresentedSize() const { return Size() * _scale; }
  [[nodiscard]] BitDepth GetBitDepth() const override { return _bitDepth; }
  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] e00::Color GetColorFromPalette(size_t index) const override { return _palette[index]; }
  [[nodiscard]] uint8_t GetClosestColor(const e00::Color &color) const override { return _palette.findClosestColorIndex(color); }
  [[nodiscard]] e00::Vec2D<e00::BitmapSizeType> Size() const override { return _size; }
  [[nodiscard]] bool IsHardwareAccelerated() const override { return true; }
  [[nodiscard]] bool SupportsOptimizedCopyFrom(const DrawableSurface &source) const override;
  [[nodiscard]] std::unique_ptr<e00::Painter> BeginDraw() override;
  [[nodiscard]] std::unique_ptr<e00::DrawableSurface> CreateOptimizedSurface(const e00::Vec2D<e00::BitmapSizeType> &size, platform::MemoryPlacement where) override;

  void SetPalette(const e00::FixedPalette &palette) override {
    _palette = palette;
    paletteChanged = true;
  }

  void ReadLineInto(
      e00::BitmapSizeType line,
      e00::BitmapSizeType startX, e00::BitmapSizeType endX,
      const TargetInformation &targetInformation, const std::span<uint8_t> &targetBuffer) const override;

  void ReadTransparencyMaskLineInto(e00::BitmapSizeType line, e00::BitmapSizeType startX, e00::BitmapSizeType endX, const std::span<uint8_t> &targetBuffer) const override;

  id<MTLTexture> PresentTexture() const { return _presentTexture; }
  void setPresentTexture(id<MTLTexture> pt) { _presentTexture = pt; }

  MTLTextureDescriptor *getDescriptor() const;
  bool UploadToTexture(id<MTLDevice> device);
};
}// namespace apple
