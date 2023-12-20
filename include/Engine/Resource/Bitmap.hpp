#pragma once

#include <array>
#include <memory>
#include <system_error>

namespace e00 {
class SoftwareBitmap;

class Bitmap : public Resource {
protected:
public:
  NOT_COPYABLE(Bitmap);

  enum class BitDepth {
    INDEXED_NO_PALETTE,
    INDEXED_PALETTE,
    TRUE_COLOR_555,
    TRUE_COLOR_565,
    TRUE_COLOR_5551,
    TRUE_COLOR_888,
    TRUE_COLOR_8888,
    GRAYSCALE_8,
  };

  enum class Type {
    SOFTWARE,
    PLATFORM,
  };

  Bitmap() = default;

  ~Bitmap() override = default;

  virtual std::error_code SetPaletteColor(std::size_t index, const Color &color) { return {}; }

  virtual std::error_code SetPalette(const std::array<Color, 255> &palette) {
    if (GetBitDepth() != BitDepth::INDEXED_PALETTE) {
      return {};
    }

    for (decltype(palette.size()) i = 0; i < palette.size(); i++) {
      if (auto ec = SetPaletteColor(i, palette.at(i)))
        return ec;
    }

    return {};
  }

  /**
   * If this is a
   * @return Software bitmap pointer of `this` or nullptr
   */
  SoftwareBitmap *GetSoftwareBitmap();

  virtual Color GetPaletteColor(uint8_t index) const { return {}; };

  virtual Vec2D<uint16_t> Size() const = 0;

  virtual Type GetType() const = 0;

  virtual BitDepth GetBitDepth() const = 0;

  // Slow per pixel access
  virtual Color GetPixel(const Vec2D<uint16_t> &position) = 0;

  virtual void SetPixel(const Vec2D<uint16_t> &position, const Color &color) = 0;
  /*
  virtual void line(Vec2D<uint16_t> from, Vec2D<uint16_t> to, int color) = 0;
  virtual void rectfill(Vec2D<uint16_t> from, Vec2D<uint16_t> to, int color) = 0;
  virtual void triangle(Vec2D<uint16_t> a, Vec2D<uint16_t> b, Vec2D<uint16_t> c, int color) = 0;
  virtual void clear_to_color(int color) = 0;
  virtual void polygon(int vertices, const int *points, int color) = 0;
  virtual void rect(Vec2D<uint16_t> from, Vec2D<uint16_t> to, int color) = 0;
  virtual void circle(Vec2D<uint16_t> pos, int radius, int color) = 0;
  virtual void circlefill(Vec2D<uint16_t> pos, int radius, int color) = 0;
  virtual void ellipse(Vec2D<uint16_t> pos, int rx, int ry, int color) = 0;
  virtual void ellipsefill(Vec2D<uint16_t> pos, int rx, int ry, int color) = 0;
  virtual void arc(Vec2D<uint16_t> pos, fixed ang1, fixed ang2, int r, int color) = 0;
  virtual void spline(const int points[8], int color) = 0;
  virtual void floodfill(Vec2D<uint16_t> pos, int color) = 0;
  */
  /**
   * Blits a section from `source` to this bitmap
   *
   * @param source which bitmap to copy from
   * @param source_rect the source rectangle to copy from in the source bitmap
   * @param destination_position the top left coordinates to blit to in this bitmap
   * @return any errors
   */
  std::error_code Blit(const Bitmap &source, const RectT<uint16_t> &source_rect, const Vec2D<uint16_t> &destination_position);

  /*
  virtual void set_clip() = 0;
  virtual void acquire() = 0;
  virtual void release() = 0;

  virtual void draw_sprite(std::shared_ptr<Bitmap> sprite, int x, int y) = 0;
  virtual void draw_sprite_v_flip(std::shared_ptr<Bitmap> sprite, int x, int y) = 0;
  virtual void draw_sprite_h_flip(std::shared_ptr<Bitmap> sprite, int x, int y) = 0;
  virtual void draw_sprite_vh_flip(std::shared_ptr<Bitmap> sprite, int x, int y) = 0;
  virtual void draw_character(std::shared_ptr<Bitmap> sprite, Vec2D<uint16_t> pos, int color, int bg) = 0;
  virtual void draw_glyph(const struct FONT_GLYPH *glyph, Vec2D<uint16_t> pos, int color, int bg) = 0;

  virtual void blit_from_memory(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_to_memory(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_from_system(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_to_system(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_to_self(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_to_self_forward(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_to_self_backward(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void blit_between_formats(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;

  virtual void masked_blit(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = 0;
  virtual void pivot_scaled_sprite_flip(std::shared_ptr<Bitmap> sprite, fixed x, fixed y, fixed cx, fixed cy, fixed angle, fixed scale, int v_flip) = 0;
  virtual void do_stretch_blit(std::shared_ptr<Bitmap> source, std::shared_ptr<Bitmap> dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked) = 0;
  virtual void draw_gouraud_sprite(std::shared_ptr<Bitmap> sprite, Vec2D<uint16_t> pos, int c1, int c2, int c3, int c4) = 0;
  virtual void draw_sprite_end() = 0;
  virtual void blit_end() = 0;

  virtual void polygon3d(int type, std::shared_ptr<Bitmap> texture, int vc, V3D *vtx[]) = 0;
  virtual void polygon3d_f(int type, std::shared_ptr<Bitmap> texture, int vc, V3D_f *vtx[]) = 0;
  virtual void triangle3d(int type, std::shared_ptr<Bitmap> texture, V3D *v1, V3D *v2, V3D *v3) = 0;
  virtual void triangle3d_f(int type, std::shared_ptr<Bitmap> texture, V3D_f *v1, V3D_f *v2, V3D_f *v3) = 0;
  virtual void quad3d(int type, std::shared_ptr<Bitmap> texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4) = 0;
  virtual void quad3d_f(int type, std::shared_ptr<Bitmap> texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4) = 0;

  virtual void draw_sprite_ex(std::shared_ptr<Bitmap> sprite, Vec2D<uint16_t> pos, int mode, int flip) = 0;
  */
};

class SoftwareBitmap : public Bitmap {
  Vec2D<uint16_t> _size;
  BitDepth _bpp;
  std::vector<uint8_t> _bitmapData;

  [[nodiscard]] auto ToLinear(const Vec2D<uint16_t>& v) const {
    return (v.y * _size.x * PixelStride()) + (v.x * PixelStride());
  }

public:
  NOT_COPYABLE(SoftwareBitmap);

  SoftwareBitmap(const Vec2D<uint16_t> &size, Bitmap::BitDepth depth);

  ~SoftwareBitmap() override;

  [[nodiscard]] Vec2D<uint16_t> Size() const override { return _size; }

  [[nodiscard]] Type GetType() const override { return Type::SOFTWARE; }

  [[nodiscard]] BitDepth GetBitDepth() const override { return _bpp; }

  [[nodiscard]] uint8_t PixelStride() const noexcept {
    switch (_bpp) {
        // 8 bits
      case BitDepth::GRAYSCALE_8:
      case BitDepth::INDEXED_NO_PALETTE:
      case BitDepth::INDEXED_PALETTE:
        return 1;

        // 16 bits
      case BitDepth::TRUE_COLOR_555:
      case BitDepth::TRUE_COLOR_565:
      case BitDepth::TRUE_COLOR_5551:
        return 2;

        // 24 bits
      case BitDepth::TRUE_COLOR_888:
        return 3;

        // 32 bits
      case BitDepth::TRUE_COLOR_8888:
        return 4;
    }
  }

  Color GetPixel(const Vec2D<uint16_t> &position) override {
    const auto& index = _bitmapData.at(ToLinear(position));

  }

  void SetPixel(const Vec2D<uint16_t> &position, const Color &color) override {
    const auto& index = _bitmapData.at(ToLinear(position));

  }
};

}// namespace e00
