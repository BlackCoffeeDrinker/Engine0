
#pragma once
#include "Engine/Config.hpp"
#include "Engine/Math/Color.hpp"
#include "Engine/Math/Rect.hpp"
#include "Engine/Math/Vec2D.hpp"
#include "Engine/Platform/DrawableSurface.hpp"
#include <span>

namespace e00 {

/**
 * Painter provides highly optimized functions to do most of the drawing programs require. It can draw everything from
 * simple lines to bitmaps.
 * 
 * While there are defaults for all methods except put point, it's highly recommended to extend all methods to provide
 * optimal performance
 */
class Painter {
protected:
  Painter() = default;

  // Outline
  enum class PenStyle {
    NoPen,
    TransparentPen,
    SolidLineColor,
    SolidLineIndex,
  };

  // Fill
  enum class BrushStyle {
    NoBrush,
    TransparentBrush,
    SolidBrushColor,
    SolidBrushIndex,
  };

  BrushStyle _brushStyle = BrushStyle::NoBrush;
  uint8_t _brushIndex = 0;
  Color _brushColor;

  PenStyle _penStyle = PenStyle::NoPen;
  uint16_t _penWidth = 0;
  uint8_t _penIndex = 0;
  Color _penColor;

public:
  virtual ~Painter();// Acts as "EndPaint()", restoring hardware modes automatically

  void SetNoPen() { _penStyle = PenStyle::NoPen; }
  void SetTransparentPen() { _penStyle = PenStyle::TransparentPen; }
  void SetNoBrush() { _brushStyle = BrushStyle::NoBrush; }
  void SetTransparentBrush() { _brushStyle = BrushStyle::TransparentBrush; }

  void SetPenSolid(uint16_t penWidth, const Color &color) {
    _penWidth = penWidth;
    _penColor = color;
    _penStyle = PenStyle::SolidLineColor;
  }

  void SetPenSolid(uint16_t penWidth, uint8_t index) {
    _penWidth = penWidth;
    _penIndex = index;
    _penStyle = PenStyle::SolidLineIndex;
  }

  void SetBrushColor(const Color &color) {
    _brushColor = color;
    _brushStyle = BrushStyle::SolidBrushColor;
  }

  void SetBrushIndex(uint8_t index) {
    _brushIndex = index;
    _brushStyle = BrushStyle::SolidBrushIndex;
  }

  /**
   * TODO: Remove ?
   * @return target information
   */
  [[nodiscard]] virtual DrawableSurface::TargetInformation GetTargetInformation() const = 0;
  [[nodiscard]] virtual BitmapSize GetDrawableSize() const = 0;

  // DrawPoint and DrawPoints should only be used for small numbers of points or debugging
  // Color of the points are determined by the current pen settings
  virtual void DrawPoint(const Vec2D<BitmapSizeType> &pos) = 0;
  virtual void DrawPoints(const std::span<Vec2D<BitmapSizeType>> &points) {
    for (const auto &point: points) { DrawPoint(point); }
  }
  virtual void DrawPoints(std::initializer_list<Vec2D<BitmapSizeType>> points) {
    for (const auto &point: points) { DrawPoint(point); }
  }

  // Color of the lines are determined by the current pen settings
  virtual void DrawLine(const Vec2D<BitmapSizeType> &start, const Vec2D<BitmapSizeType> &end) = 0;

  // Outline is determined by the current pen settings, and fill is determined by the current brush settings
  virtual void DrawEllipse(const RectT<BitmapSizeType> &rect) = 0;

  // Outline is determined by the current pen settings, and fill is determined by the current brush settings
  virtual void DrawRect(const RectT<BitmapSizeType> &rect) = 0;

  // High-speed line blitting interface, pen and brush aren't applied
  virtual void BlitRawLine(BitmapSizeType line,
                           BitmapSizeType startX, BitmapSizeType endX,
                           const std::span<const uint8_t> &data,
                           const DrawableSurface::TargetInformation &dataFormatting) = 0;

  /**
   * 
   * @param line Line number
   * @param startX Start X coordinate
   * @param endX End X coordinate
   * @param data Bitmap data in destination format
   * @param mask Mask data in DEPTH_1 (1 byte = 8 pixels)
   * @param dataFormatting Format in data
   */
  virtual void BlitMaskedLine(BitmapSizeType line,
                              BitmapSizeType startX, BitmapSizeType endX,
                              const std::span<const uint8_t> &data, const std::span<const uint8_t> &mask,
                              const DrawableSurface::TargetInformation &dataFormatting) = 0;

  // High-speed block blitting interface, pen and brush aren't applied
  virtual void BlitSurface(const DrawableSurface &src,
                           RectT<BitmapSizeType> srcRect,
                           Vec2D<BitmapSizeType> dstPos) = 0;
};
}// namespace e00
