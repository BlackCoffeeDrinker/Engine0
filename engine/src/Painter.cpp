#include "PrivateInclude.hpp"
#include "Painter_PaintDevice.hpp"

namespace e00 {

Painter::Painter() = default;

Painter::Painter(const Bitmap &targetBitmap) :  {
}

void Painter::setPen(PenStyle penStyle, uint16_t penWidth, const Color &color) {
}

void Painter::setNoBrush() {
}

void Painter::setBrushColor(const Color &color) {
}

void Painter::setBrushBitmap(const Bitmap &bitmap) {
}

void Painter::fillRect(const RectT<uint16_t> &rectangle) {
}

void Painter::drawBitmap(const RectT<uint16_t> &targetDestination, const RectT<uint16_t> &source, const Bitmap &bitmap) {
}

void Painter::drawLine(const Vec2D<uint16_t> &point1, const Vec2D<uint16_t> &point2) {
}


}// namespace e00
