#include "BitmapLoader.hpp"

namespace e00::impl {
BitmapLoader::BitmapLoader() = default;

bool BitmapLoader::CanLoad(const LoadContext &context) {
  return false;
}

ResourceLoader::Result BitmapLoader::ReadLoad(const LoadContext &context) {
  return make_error_code(errc::invalid_argument);
}

}// namespace e00::impl
