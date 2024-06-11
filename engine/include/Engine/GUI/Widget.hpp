#pragma once

namespace e00 {
class Widget {
  Widget *_parent;
  std::list<Widget*> _children;

  bool _needs_drawing;

  RectT<uint16_t> _rect;
  bool _has_background;
  Color _background;

public:
  Widget(Widget *parent);

  virtual ~Widget();

  virtual void SetSize(const Vec2D<uint16_t>& size) {
      _rect.size = size;
      _needs_drawing = true;
  }

  virtual void SetPosition(const Vec2D<uint16_t>& position) {
      _rect.origin = position;
      _needs_drawing = true;
  }

  virtual void Paint(Bitmap& bmp);
};
}// namespace e00
