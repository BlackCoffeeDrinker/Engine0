#include "BitmapLoader.hpp"
#include "lodepng.h"

namespace {
class PNGBitmap : public e00::Bitmap {
protected:
  lodepng::State png_state;
  e00::Vec2D<uint16_t> size;
  std::vector<uint8_t> image;

  PNGBitmap(lodepng::State &&state, const e00::Vec2D<uint16_t> &size, std::vector<unsigned char> data)
    : png_state(state),
      size(size),
      image(std::move(data)) {
  }

public:
  ~PNGBitmap() override = default;

  e00::Vec2D<uint16_t> Size() const override {
    return size;
  }

  Type GetType() const override {
    return Type::SOFTWARE;
  }

  void SetPixel(const e00::Vec2D<uint16_t> &position, const e00::Color &color) override {}

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return e00::Color();
  }

};

class IndexPNG : public PNGBitmap {
public:
  IndexPNG(lodepng::State &&state, const e00::Vec2D<uint16_t> &size, std::vector<unsigned char> data)
    : PNGBitmap(std::move(state), size, std::move(data)) {}

  ~IndexPNG() override = default;

  e00::Color GetPaletteColor(uint8_t index) const override {
    return png_state.info_raw.palette != nullptr && index < png_state.info_raw.palettesize
             ? e00::Color(png_state.info_raw.palette[index * 4 + 0], png_state.info_raw.palette[index * 4 + 1], png_state.info_raw.palette[index * 4 + 2])
             : e00::Color();
  }

  BitDepth GetBitDepth() const override {
    return BitDepth::INDEXED_PALETTE;
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return GetPaletteColor(image.at(position.y * size.x + position.x));
  }
};

class GrayscalePNG : public PNGBitmap {
public:
  GrayscalePNG(lodepng::State &&state, const e00::Vec2D<uint16_t> &size, std::vector<unsigned char> data)
    : PNGBitmap(std::move(state), size, std::move(data)) {}

  ~GrayscalePNG() override = default;

  BitDepth GetBitDepth() const override {
    return e00::Bitmap::BitDepth::GRAYSCALE_8;
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return e00::Color();
  }
};

class RGBPNG : public PNGBitmap {
public:
  RGBPNG(lodepng::State &&state, const e00::Vec2D<uint16_t> &size, std::vector<unsigned char> data)
    : PNGBitmap(std::move(state), size, std::move(data)) {}

  ~RGBPNG() override = default;

  BitDepth GetBitDepth() const override {
    return BitDepth::TRUE_COLOR_888;
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return e00::Color();
  }
};

class RGBAPNG : public PNGBitmap {
public:
  RGBAPNG(lodepng::State &&state, const e00::Vec2D<uint16_t> &size, std::vector<unsigned char> data)
    : PNGBitmap(std::move(state), size, std::move(data)) {}

  ~RGBAPNG() override = default;

  BitDepth GetBitDepth() const override {
    return BitDepth::TRUE_COLOR_8888;
  }

  e00::Color GetPixel(const e00::Vec2D<uint16_t> &position) override {
    return e00::Color();
  }
};
}// namespace

namespace e00::impl {
BitmapLoader::BitmapLoader() = default;

bool BitmapLoader::CanLoad(e00::Stream &stream) {
  return true;
}

ResourceLoader::Result BitmapLoader::ReadLoad(Stream &stream) {
  std::vector<uint8_t> image;
  unsigned width = 0, height = 0;
  lodepng::State state;

  state.decoder.color_convert = 0;

  // Load the image into memory and make sure it's valid
  {
    std::vector<uint8_t> png;
    png.resize(stream.stream_size());
    stream.read(stream.stream_size(), png.data());

    if (const auto error = lodepng::decode(image, width, height, state, png)) {
      _logger.Error(source_location::current(), "Failed to load bitmap {}: {}", error, lodepng_error_text(error));
      return std::make_error_code(std::errc::broken_pipe);
    }
  }

  // Make the specific holder for this PNG
  std::unique_ptr<PNGBitmap> pngsoftware;

  // Make the correct subclass for this PNG
  switch (state.info_raw.colortype) {
    case LCT_RGB:
      // RGB modes are only supported in 8bpp
      if (state.info_raw.bitdepth != 8) {
        _logger.Error(e00::source_location::current(), "PNG RGB is only support in 8bpp, file is in {} bpp", state.info_raw.bitdepth);
        return std::make_error_code(std::errc::invalid_argument);
      }

      pngsoftware = std::make_unique<RGBPNG>(std::move(state),
        Vec2D<uint16_t>(width, height),
        std::move(image));
      break;

    case LCT_RGBA:
      // RGBA modes are only supported in 8bpp
      if (state.info_raw.bitdepth != 8) {
        _logger.Error(e00::source_location::current(), "PNG RGBA is only support in 8bpp, file is in {} bpp", state.info_raw.bitdepth);
        return std::make_error_code(std::errc::invalid_argument);
      }

      pngsoftware = std::make_unique<RGBAPNG>(std::move(state),
        Vec2D<uint16_t>(width, height),
        std::move(image));
      break;

    case LCT_PALETTE:
      // Palettized PNG
      _logger.Verbose(source_location::current(), "Indexed PNG with bitdepth {} and {} colors", state.info_raw.bitdepth, state.info_raw.palettesize);
      pngsoftware = std::make_unique<IndexPNG>(std::move(state),
        Vec2D<uint16_t>(width, height),
        std::move(image));
      break;

    case LCT_GREY:
    case LCT_GREY_ALPHA:
      // We don't support grayscale except 8bit
      if (state.info_raw.bitdepth != 8) {
        _logger.Error(e00::source_location::current(), "PNG Grayscale is only support in 8bpp");
        return std::make_error_code(std::errc::invalid_argument);
      }

      pngsoftware = std::make_unique<GrayscalePNG>(std::move(state),
        Vec2D<uint16_t>(width, height),
        std::move(image));
      break;

    default:
      // Color type needs to be valid
      _logger.Error(e00::source_location::current(), "PNG Color type is currently not supported");
      return std::make_error_code(std::errc::invalid_argument);
  }

  return { std::move(pngsoftware) };
}

}// namespace e00::impl
