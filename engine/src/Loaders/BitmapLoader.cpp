#include "BitmapLoader.hpp"
#include "lodepng.h"

namespace e00::impl {
BitmapLoader::BitmapLoader() = default;

bool BitmapLoader::CanLoad(const LoadContext& context) {
  return false;
}

ResourceLoader::Result BitmapLoader::ReadLoad(const LoadContext& context) {
  return std::make_error_code(std::errc::invalid_argument);
}

}// namespace e00::impl
