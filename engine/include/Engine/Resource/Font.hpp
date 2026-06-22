#pragma once

#include <array>
#include <memory>
#include <vector>

namespace e00 {
class Font : public Resource {
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
  uint16_t _height{};
  std::unique_ptr<GlyphCollection> _glyphs;

public:
  static Font &DefaultFont();

  Font() = default;
  ~Font() override = default;
  virtual uint16_t FontHeight() { return _height; }
  virtual uint16_t CharLength(uint16_t ch) const = 0;
  virtual uint16_t TextLength(const std::string_view &text);

  [[nodiscard]] type_t Type() const noexcept override { return type_id<Font>(); }

  virtual uint16_t RenderChar(std::string_view::value_type ch, Color fg, Color bg, Painter &painter, const Vec2D<uint16_t> &position) = 0;

  virtual void Render(const std::string_view &text, Color fg, Color bg, Painter &painter, const RectT<uint16_t> &rect);
};

class FontMonochrome : public Font {

  [[nodiscard]] const Glyph &FindGlyph(uint16_t ch) const;

public:
  FontMonochrome();

  ~FontMonochrome() noexcept override;

  uint16_t CharLength(uint16_t ch) const override;
  uint16_t RenderChar(std::string_view::value_type ch, Color fg, Color bg, Painter &bmp, const Vec2D<uint16_t> &position) override;
};
}// namespace e00
