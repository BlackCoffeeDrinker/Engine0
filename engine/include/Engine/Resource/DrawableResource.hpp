#pragma once

#include <Engine/Platform/DrawableSurface.hpp>
#include <Engine/Resource.hpp>

namespace e00 {
/**
 * A loaded drawable resource 
 */
class DrawableResource : public Resource, public DrawableSurface {
  const Vec2D<BitmapSizeType> _size;
  BitDepth _bit_depth;

protected:
  DrawableResource(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth)
      : _size(size), _bit_depth(bit_depth) {}

public:
  ~DrawableResource() override = default;

  void DiscardPalette() override {
    if (_bit_depth == BitDepth::DEPTH_8) { _bit_depth = BitDepth::DEPTH_8_NO_PALETTE; }
  }
  [[nodiscard]] Vec2D<BitmapSizeType> Size() const override { return _size; }
  [[nodiscard]] BitDepth GetBitDepth() const override { return _bit_depth; }
};
}// namespace e00
