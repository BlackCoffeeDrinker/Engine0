#pragma once

#include <memory>

namespace e00 {
class Font {
public:
  struct Glyph {
    uint16_t width;
    uint16_t height;
    std::array<uint8_t, 64> dat;
  };

  struct GlyphCollection {
    uint16_t start;
    uint16_t end;
    std::vector<Glyph> glyphs;

    std::unique_ptr<GlyphCollection> next;

    [[nodiscard]] const GlyphCollection *FindInRange(uint16_t ch) const;
  };

protected:
  uint16_t _height;
  std::unique_ptr<GlyphCollection> _glyphs;

public:
  static Font &DefaultFont();

  Font() = default;
  virtual ~Font() = default;
  virtual uint16_t FontHeight() { return _height; }
  virtual uint16_t CharLength(uint16_t ch) = 0;
  virtual uint16_t TextLength(const std::string_view &text);

  /**
   * Renders a glyph to a bitmap
   *
   * @param ch Character to render
   * @param fg foreground color
   * @param bg background color
   * @param bmp bitmap to render to
   * @param position where to put this character
   * @return the width of the rendered
   */
  virtual uint16_t RenderChar(std::string_view::value_type ch, Color fg, Color bg, Bitmap &bmp, const Vec2D<uint16_t> &position) = 0;

  virtual void Render(const std::string_view &text, Color fg, Color bg, Bitmap &bmp, const Vec2D<uint16_t> &position);
};

class FontMonochrome : public Font
  , public Resource {

  [[nodiscard]] const Font::Glyph &FindGlyph(uint16_t ch) const;

public:
  FontMonochrome();

  ~FontMonochrome() override;

  uint16_t CharLength(uint16_t ch) override;
  uint16_t RenderChar(std::string_view::value_type ch, Color fg, Color bg, Bitmap &bmp, const Vec2D<uint16_t> &position) override;
};
}// namespace e00
