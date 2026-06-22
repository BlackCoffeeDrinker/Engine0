#pragma once

#include <string>
#include <string_view>

#include "Widget.hpp"

namespace e00 {
class Font;

/**
 * @class LabelWidget
 * @brief Simple widget that displays a single text label.
 */
class LabelWidget : public Widget {
  std::string _text;
  Font *_font;
  Color _foreground_color;
  Color _background_color;

  void UpdateMinimumSize();

protected:
  void ComputeSize() override;

public:
  explicit LabelWidget();
  explicit LabelWidget(std::string text);

  void SetText(std::string text);
  [[nodiscard]] std::string_view Text() const noexcept { return _text; }

  void SetFont(Font &font);
  [[nodiscard]] Font &GetFont() const noexcept;

  void SetForegroundColor(const Color &color) noexcept { _foreground_color = color; }
  [[nodiscard]] const Color &ForegroundColor() const noexcept { return _foreground_color; }

  void SetBackgroundColor(const Color &color) noexcept { _background_color = color; }
  [[nodiscard]] const Color &BackgroundColor() const noexcept { return _background_color; }

  void Paint(Painter &painterObj) override;
};
}// namespace e00
