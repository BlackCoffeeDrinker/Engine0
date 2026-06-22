
#include <Engine.hpp>

namespace e00 {

LabelWidget::LabelWidget()
    : _font(&Font::DefaultFont()),
      _foreground_color(255, 255, 255),
      _background_color(0, 0, 0) {
  UpdateMinimumSize();
}

LabelWidget::LabelWidget(std::string text)
    : _text(std::move(text)),
      _font(&Font::DefaultFont()),
      _foreground_color(255, 255, 255),
      _background_color(0, 0, 0) {
  UpdateMinimumSize();
}

void LabelWidget::SetText(std::string text) {
  if (_text == text) {
    return;
  }

  _text = std::move(text);
  UpdateMinimumSize();
}

void LabelWidget::SetFont(Font &font) {
  if (_font == &font) {
    return;
  }

  _font = &font;
  UpdateMinimumSize();
}

Font &LabelWidget::GetFont() const noexcept {
  return *_font;
}

void LabelWidget::UpdateMinimumSize() {
  auto &font = GetFont();

  SetMinimumSize({
      font.TextLength(_text),
      font.FontHeight(),
  });
}

void LabelWidget::ComputeSize() {
  SetComputedSize(MinimumSize());
}

void LabelWidget::Paint(Painter &painterObj) {
  Widget::Paint(painterObj);

  if (_text.empty()) {
    return;
  }

  const auto paintRect = AbsoluteComputedRect();
  if (paintRect.size.x == 0 || paintRect.size.y == 0) {
    return;
  }

  GetFont().Render(_text, _foreground_color, _background_color, painterObj, paintRect);
}

}// namespace e00
