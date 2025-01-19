#include "BitmapLoader.hpp"
#include "lodepng.h"

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

  // Load the image into memory and make sure it is valid
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
  std::unique_ptr<Bitmap> pngsoftware;

  

  return { std::move(pngsoftware) };
}

}// namespace e00::impl
