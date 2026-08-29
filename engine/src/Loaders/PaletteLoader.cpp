
#include "PaletteLoader.h"

#include <charconv>

#include "IniParser.hpp"

namespace e00::impl {
bool PaletteLoader::CanLoad(const LoadContext& context) {
  return true;
}

ResourceLoader::Result PaletteLoader::ReadLoad(const LoadContext& context) {
  auto palette = std::make_unique<FixedPalette>();
  
  const auto ec = IniParser::Parse(context.stream, [&](const IniParser::Item &item) -> std::error_code {
    if (item.category == "palette") {
      if (item.key == "colors") {
        size_t psize = 0;
        if (const auto res = std::from_chars(item.value.begin(), item.value.begin() + item.value.size(), psize); res.ec == std::errc()) {
          if (psize <= 0)
            return std::make_error_code(std::errc::invalid_argument);

          if (psize > FixedPalette::MAX_SIZE)
            return std::make_error_code(std::errc::invalid_argument);

          palette->resize(psize);
        } else {
          return std::make_error_code(std::errc::invalid_argument);
        }
      }
    }
    if (item.category == "colors") {
      // Key is Palette color index, value is R G B
      size_t index = 0;
      if (const auto res = std::from_chars(item.key.begin(), item.key.begin() + item.key.size(), index);
          res.ec == std::errc()) {
        if (index >= palette->size()) {
          return std::make_error_code(std::errc::invalid_argument);
        }

        Color color;
        if (!ParseRGB(item.value, color)) {
          return std::make_error_code(std::errc::invalid_argument);
        }
        palette->set(index, color);
      }
    }
    return {};
  });

  if (ec)
    return ec;

  return palette;
}

}// namespace e00::impl
