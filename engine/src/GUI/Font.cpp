#include <Engine.hpp>

namespace {
/* standard ASCII characters (0x20 to 0x7F) */
const e00::Font::Glyph f_0x20 = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x21 = { 8, 8, { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00 } };
const e00::Font::Glyph f_0x22 = { 8, 8, { 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x23 = { 8, 8, { 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00 } };
const e00::Font::Glyph f_0x24 = { 8, 8, { 0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00 } };
const e00::Font::Glyph f_0x25 = { 8, 8, { 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00 } };
const e00::Font::Glyph f_0x26 = { 8, 8, { 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00 } };
const e00::Font::Glyph f_0x27 = { 8, 8, { 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x28 = { 8, 8, { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00 } };
const e00::Font::Glyph f_0x29 = { 8, 8, { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00 } };
const e00::Font::Glyph f_0x2A = { 8, 8, { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00 } };
const e00::Font::Glyph f_0x2B = { 8, 8, { 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00 } };
const e00::Font::Glyph f_0x2C = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 } };
const e00::Font::Glyph f_0x2D = { 8, 8, { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x2E = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 } };
const e00::Font::Glyph f_0x2F = { 8, 8, { 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00 } };
const e00::Font::Glyph f_0x30 = { 8, 8, { 0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00 } };
const e00::Font::Glyph f_0x31 = { 8, 8, { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00 } };
const e00::Font::Glyph f_0x32 = { 8, 8, { 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00 } };
const e00::Font::Glyph f_0x33 = { 8, 8, { 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x34 = { 8, 8, { 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00 } };
const e00::Font::Glyph f_0x35 = { 8, 8, { 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x36 = { 8, 8, { 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x37 = { 8, 8, { 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00 } };
const e00::Font::Glyph f_0x38 = { 8, 8, { 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x39 = { 8, 8, { 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00 } };
const e00::Font::Glyph f_0x3A = { 8, 8, { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 } };
const e00::Font::Glyph f_0x3B = { 8, 8, { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 } };
const e00::Font::Glyph f_0x3C = { 8, 8, { 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00 } };
const e00::Font::Glyph f_0x3D = { 8, 8, { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x3E = { 8, 8, { 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00 } };
const e00::Font::Glyph f_0x3F = { 8, 8, { 0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00 } };
const e00::Font::Glyph f_0x40 = { 8, 8, { 0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00 } };
const e00::Font::Glyph f_0x41 = { 8, 8, { 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00 } };
const e00::Font::Glyph f_0x42 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00 } };
const e00::Font::Glyph f_0x43 = { 8, 8, { 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00 } };
const e00::Font::Glyph f_0x44 = { 8, 8, { 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00 } };
const e00::Font::Glyph f_0x45 = { 8, 8, { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00 } };
const e00::Font::Glyph f_0x46 = { 8, 8, { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00 } };
const e00::Font::Glyph f_0x47 = { 8, 8, { 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00 } };
const e00::Font::Glyph f_0x48 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00 } };
const e00::Font::Glyph f_0x49 = { 8, 8, { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
const e00::Font::Glyph f_0x4A = { 8, 8, { 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x4B = { 8, 8, { 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00 } };
const e00::Font::Glyph f_0x4C = { 8, 8, { 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00 } };
const e00::Font::Glyph f_0x4D = { 8, 8, { 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00 } };
const e00::Font::Glyph f_0x4E = { 8, 8, { 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00 } };
const e00::Font::Glyph f_0x4F = { 8, 8, { 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00 } };
const e00::Font::Glyph f_0x50 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00 } };
const e00::Font::Glyph f_0x51 = { 8, 8, { 0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00 } };
const e00::Font::Glyph f_0x52 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00 } };
const e00::Font::Glyph f_0x53 = { 8, 8, { 0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00 } };
const e00::Font::Glyph f_0x54 = { 8, 8, { 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
const e00::Font::Glyph f_0x55 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00 } };
const e00::Font::Glyph f_0x56 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00 } };
const e00::Font::Glyph f_0x57 = { 8, 8, { 0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
const e00::Font::Glyph f_0x58 = { 8, 8, { 0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00 } };
const e00::Font::Glyph f_0x59 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00 } };
const e00::Font::Glyph f_0x5A = { 8, 8, { 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00 } };
const e00::Font::Glyph f_0x5B = { 8, 8, { 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00 } };
const e00::Font::Glyph f_0x5C = { 8, 8, { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00 } };
const e00::Font::Glyph f_0x5D = { 8, 8, { 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00 } };
const e00::Font::Glyph f_0x5E = { 8, 8, { 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x5F = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF } };
const e00::Font::Glyph f_0x60 = { 8, 8, { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x61 = { 8, 8, { 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00 } };
const e00::Font::Glyph f_0x62 = { 8, 8, { 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00 } };
const e00::Font::Glyph f_0x63 = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x64 = { 8, 8, { 0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00 } };
const e00::Font::Glyph f_0x65 = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
const e00::Font::Glyph f_0x66 = { 8, 8, { 0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00 } };
const e00::Font::Glyph f_0x67 = { 8, 8, { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
const e00::Font::Glyph f_0x68 = { 8, 8, { 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00 } };
const e00::Font::Glyph f_0x69 = { 8, 8, { 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
const e00::Font::Glyph f_0x6A = { 8, 8, { 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78 } };
const e00::Font::Glyph f_0x6B = { 8, 8, { 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00 } };
const e00::Font::Glyph f_0x6C = { 8, 8, { 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
const e00::Font::Glyph f_0x6D = { 8, 8, { 0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00 } };
const e00::Font::Glyph f_0x6E = { 8, 8, { 0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00 } };
const e00::Font::Glyph f_0x6F = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00 } };
const e00::Font::Glyph f_0x70 = { 8, 8, { 0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0 } };
const e00::Font::Glyph f_0x71 = { 8, 8, { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E } };
const e00::Font::Glyph f_0x72 = { 8, 8, { 0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00 } };
const e00::Font::Glyph f_0x73 = { 8, 8, { 0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00 } };
const e00::Font::Glyph f_0x74 = { 8, 8, { 0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00 } };
const e00::Font::Glyph f_0x75 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00 } };
const e00::Font::Glyph f_0x76 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00 } };
const e00::Font::Glyph f_0x77 = { 8, 8, { 0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
const e00::Font::Glyph f_0x78 = { 8, 8, { 0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00 } };
const e00::Font::Glyph f_0x79 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
const e00::Font::Glyph f_0x7A = { 8, 8, { 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00 } };
const e00::Font::Glyph f_0x7B = { 8, 8, { 0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00 } };
const e00::Font::Glyph f_0x7C = { 8, 8, { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 } };
const e00::Font::Glyph f_0x7D = { 8, 8, { 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00 } };
const e00::Font::Glyph f_0x7E = { 8, 8, { 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const e00::Font::Glyph f_0x7F = { 8, 8, { 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00 } };


/* list of ASCII characters */
const std::vector<e00::Font::Glyph> ascii_data{ { f_0x20, f_0x21, f_0x22, f_0x23, f_0x24, f_0x25, f_0x26, f_0x27, f_0x28, f_0x29, f_0x2A, f_0x2B, f_0x2C, f_0x2D, f_0x2E, f_0x2F, f_0x30, f_0x31, f_0x32, f_0x33, f_0x34, f_0x35, f_0x36, f_0x37, f_0x38, f_0x39, f_0x3A, f_0x3B, f_0x3C, f_0x3D, f_0x3E, f_0x3F, f_0x40, f_0x41, f_0x42, f_0x43, f_0x44, f_0x45, f_0x46, f_0x47, f_0x48, f_0x49, f_0x4A, f_0x4B, f_0x4C, f_0x4D, f_0x4E, f_0x4F, f_0x50, f_0x51, f_0x52, f_0x53, f_0x54, f_0x55, f_0x56, f_0x57, f_0x58, f_0x59, f_0x5A, f_0x5B, f_0x5C, f_0x5D, f_0x5E, f_0x5F, f_0x60, f_0x61, f_0x62, f_0x63, f_0x64, f_0x65, f_0x66, f_0x67, f_0x68, f_0x69, f_0x6A, f_0x6B, f_0x6C, f_0x6D, f_0x6E, f_0x6F, f_0x70, f_0x71, f_0x72, f_0x73, f_0x74, f_0x75, f_0x76, f_0x77, f_0x78, f_0x79, f_0x7A, f_0x7B, f_0x7C, f_0x7D, f_0x7E, f_0x7F } };

const e00::Font::GlyphCollection ascii_default = {
  0x20,
  0x80,
  ascii_data,
  nullptr
};

}// namespace

namespace e00 {
const Font::GlyphCollection *Font::GlyphCollection::FindInRange(uint16_t ch) const {
  if (start > ch && ch < end) return this;
  if (next) return next->FindInRange(ch);
  return nullptr;
}

Font &Font::DefaultFont() {
  static FontMonochrome defaultFont{};
  return defaultFont;
}

void Font::Render(const std::string_view &text, Color fg, Color bg, Bitmap &bmp, const Vec2D<uint16_t> &position) {
  Vec2D<uint16_t> current_positon = position;

  for (const auto& ch : text) {
    current_positon.x += RenderChar(ch, fg, bg, bmp, position);

    // Line ending
    if (current_positon.x >= bmp.Size().x) {
      current_positon.y += _height;
      current_positon.x = position.x;
    }

    // Out of bmp space
    if (current_positon.x >= bmp.Size().x) {
      return;
    }
  }
}

uint16_t Font::TextLength(const std::string_view &text) {
  uint16_t i = 0;
  for (const auto& ch : text) {
    i += CharLength(ch);
  }
  return i;
}

FontMonochrome::FontMonochrome() = default;

FontMonochrome::~FontMonochrome() noexcept = default;

const Font::Glyph &FontMonochrome::FindGlyph(uint16_t ch) const {
  if (const auto *p = _glyphs->FindInRange(ch)) {
    const auto i = ch - p->start;
    return p->glyphs.at(i);
  }

  return f_0x20;
}

uint16_t FontMonochrome::CharLength(uint16_t ch) {
  return FindGlyph(ch).width;
}

uint16_t FontMonochrome::RenderChar(std::string_view::value_type ch, Color fg, Color bg, Bitmap &bmp, const Vec2D<uint16_t> &position) {
  auto p = FindGlyph(ch);


  return p.width;
}

}// namespace e00
