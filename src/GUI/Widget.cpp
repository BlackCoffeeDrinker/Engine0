#include <Engine.hpp>

namespace e00 {
Widget::Widget(Widget *parent) : _parent(parent) {
  if (_parent) {
    _parent->_children.push_back(this);
  }
}

Widget::~Widget() {
  for (auto* w : _children) {
    delete w;
  }
}

void Widget::Paint(Bitmap &bmp) {
  if (!_needs_drawing) {
    return;
  }

  if (_parent) {

  }
}

}
