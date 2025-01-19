
#pragma once

namespace e00 {

/**
 * Painter provides highly optimized functions to do most of the drawing programs require. It can draw everything from
 * simple lines to bitmaps.
 */
class Painter {
class PaintDevice;

  PaintDevice* _target;
  
protected:
  
  Painter();

public:
  enum class PenStyle {
    NoPen,
    SolidLine,
  };

  enum class BrushStyle {
    NoBrush,
    SolidBrush,
    BitmapBrush,
  };

  explicit Painter(const Bitmap &targetBitmap);
  
  ~Painter() = default;

  /**
   * The pen defines how to draw lines and outlines; it also defines the text color.
   *
   * @param penStyle the current pen style
   * @param penWidth the pen width (can be 0)
   * @param color color of the pen
   */
  void setPen(PenStyle penStyle, uint16_t penWidth = 0, const Color &color = {});

  /**
   * Set brush to NoBrush
   */
  void setNoBrush();

  /**
   * Sets the brush type to solid and sets the color to color
   *
   * @param color the background color
   */
  void setBrushColor(const Color &color);

  /**
   * Sets the brush type to bitmap and sets the bitmap
   *
   * @param bitmap the background bitmap
   */
  void setBrushBitmap(const Bitmap &bitmap);

  /**
   * A filled rectangle has a size of rectangle.size(). A stroked rectangle has a size of rectangle.size() plus the pen width.
   *
   * @param rectangle Draws the current rectangle with the current pen and brush.
   */
  void fillRect(const RectT<uint16_t> &rectangle);

  /**
   * Draws the rectangular portion `source` of the given `bitmap` into the given target in the paint device.
   *
   * @param targetDestination where to put bitmap(source)
   * @param source the Rect in the bitmap to take from
   * @param bitmap the source bitmap
   */
  void drawBitmap(const RectT<uint16_t> &targetDestination, const RectT<uint16_t> &source, const Bitmap &bitmap);

  /**
   * Draws a line from point1 to point2.
   *
   * @param point1 the first point
   * @param point2 the second point
   */
  void drawLine(const Vec2D<uint16_t> &point1, const Vec2D<uint16_t> &point2);

  /**
   * Draws a series of connected line segments defined by the given set of points.
   *
   * @param points An array of point coordinates that define the vertices of the polylines to be drawn.
   */
  template<size_t N>
  void drawPolyLines(const std::array<Vec2D<uint16_t>, N> &points) {
    for (size_t i = 0; i < N - 1; ++i) {
      drawLine(points[i], points[i + 1]);
    }
  }

  /**
   * Draws a series of connected line segments defined by the given set of points.
   *
   * @param points A list of points that define the vertices of the polylines. The points are connected in sequence by straight lines.
   */
  void drawPolyLines(const std::vector<Vec2D<uint16_t>> &points) {
    for (size_t i = 0; i < points.size() - 1; ++i) {
      drawLine(points[i], points[i + 1]);
    }
  }
};

}// namespace e00
