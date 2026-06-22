#pragma once
#include "Engine/Config.hpp"
#include "Engine/Math/Vec2D.hpp"
#include "Engine/Resource/Palette.hpp"
#include <memory>
#include <span>
#include <system_error>

namespace e00 {
class WritableStream;
class Bitmap;
class Painter;

class DrawableSurface {
public:
  enum class BitDepth {
    DEPTH_1,           // 1-bit monochrome, (8 pixels packed into a `uint8_t`)
    DEPTH_8,           // 8-bit indexed color
    DEPTH_8_NO_PALETTE,// 8-bit indexed color, no palette
    DEPTH_16,          // 16-bit "high-color"
    DEPTH_32,          // 32-bit "true-color"

    DEPTH_INVALID
  };

  struct RGBInfo {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    constexpr RGBInfo() = default;
    constexpr RGBInfo(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
    constexpr RGBInfo(const RGBInfo &rgb_info) = default;
    constexpr RGBInfo(RGBInfo &&rgb_info) = default;
    constexpr RGBInfo &operator=(const RGBInfo &rgb_info) = default;
    constexpr RGBInfo &operator=(RGBInfo &&rgb_info) = default;

    [[nodiscard]] constexpr bool empty() const { return red == 0 && green == 0 && blue == 0; }

    bool operator==(const RGBInfo &rgb_info) const = default;
  };

  struct TargetInformation {
    BitDepth bit_depth;

    // Only valid for 1-bit and 8-bit surfaces
    const FixedPalette *palette = nullptr;

    // Only valid for 16-bit and 32-bit surfaces
    RGBInfo shift;
    RGBInfo mask;
  };

  virtual ~DrawableSurface();

  [[nodiscard]] virtual type_t Type() const = 0;
  [[nodiscard]] virtual Vec2D<BitmapSizeType> Size() const = 0;
  [[nodiscard]] virtual BitDepth GetBitDepth() const = 0;
  [[nodiscard]] virtual size_t GetNumberOfColorsInPalette() const { return 0; };
  [[nodiscard]] virtual Color GetColorFromPalette(size_t index) const { return {}; }
  [[nodiscard]] virtual uint8_t GetClosestColor(const Color &color) const { return 0; }

  /**
   * If this surface has a palette, discard it
   * Only works for surfaces where GetBitDepth() returns a DEPTH_8.
   * After calling this function, GetBitDepth() will return DEPTH_8_NO_PALETTE.
   */
  virtual void DiscardPalette() = 0;
  
  [[nodiscard]] bool HasSamePalette(const DrawableSurface &other) const {
    // If either this or the other surface is 8-bit with no palette, they are considered to have the same palette
    if (GetBitDepth() == BitDepth::DEPTH_8_NO_PALETTE || other.GetBitDepth() == BitDepth::DEPTH_8_NO_PALETTE) {
      return true;
    }

    const auto maxColors = GetNumberOfColorsInPalette();

    if (maxColors != other.GetNumberOfColorsInPalette()) {
      return false;
    }

    for (size_t i = 0; i < maxColors; ++i) {
      if (GetColorFromPalette(i) != other.GetColorFromPalette(i)) {
        return false;
      }
    }

    return true;
  }

  /**
   * Check if this surface is backed by VRAM.
   * ReadLineInto might be slow and can be copied into another surface of the 
   * same type optimally
   * 
   * @return true if this surface is hardware accelerated (e.g., in VRAM) 
   */
  [[nodiscard]] virtual bool IsHardwareAccelerated() const { return false; }

  /**
   * 
   * @param source the source buffer
   * @return true if this surface can efficiently copy from the `source` surface
   */
  [[nodiscard]] virtual bool SupportsOptimizedCopyFrom(const DrawableSurface &source) const { return false; }

  /**
   * Acquires the active rendering brush engine for this surface.
   * The returned Painter manages hardware registers, banking, or modern context streams.
   * 
   * @return the painter for this surface
   */
  [[nodiscard]] virtual std::unique_ptr<Painter> BeginDraw() = 0;

  /**
   * Universal Extraction Bridge:
   * Forces the surface to decode its internal representation (whether it's VRAM or a GUI buffer) 
   * and stream it into a flat, raw system memory byte buffer.
   * 
   * The size of the target buffer is the stride of the target information; it must be large enough
   * to hold the entire line of data, considering the target format's bit depth and any padding.
   * 
   * @param line the line to read from
   * @param startX the start X 
   * @param endX the end X
   * @param targetInformation the destination format information
   * @param targetBuffer where to write the data
   */
  virtual void ReadLineInto(
      BitmapSizeType line,
      BitmapSizeType startX, BitmapSizeType endX,
      const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const = 0;


  // For debugging
  std::error_code SaveToBMP(WritableStream &writableStream) const;
};

inline uint16_t DepthEnumToBits(DrawableSurface::BitDepth depth) {
  switch (depth) {
    case DrawableSurface::BitDepth::DEPTH_1: return 1;
    case DrawableSurface::BitDepth::DEPTH_8:
    case DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE: return 8;
    case DrawableSurface::BitDepth::DEPTH_16: return 16;
    case DrawableSurface::BitDepth::DEPTH_32: return 32;
    default: return 0;
  }
  return 0;
}
}// namespace e00
