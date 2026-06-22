
#include "PaletteLoader.h"

#include <charconv>

#include "IniParser.hpp"

namespace {
bool ParseRGB(const std::string_view &sv, e00::Color &color_out) {
  const char *p = sv.data();
  const char *end = p + sv.size();

  // --- HEX MODE ----------------------------------------------------------
  if (!sv.empty() && sv[0] == '#') {
    p++;// skip '#'
    size_t hex_len = end - p;
    auto hexval = [](char c) -> int {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
      if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
      return -1;
    };

    auto hex_to_byte = [&](char hi, char lo) -> int {
      const auto h = hexval(hi);
      const auto l = hexval(lo);
      if (h < 0 || l < 0) return 0xFF + 1;// invalid marker
      return (h << 4) | l;
    };

    if (hex_len == 6) {
      // #RRGGBB
      const auto r = hex_to_byte(p[0], p[1]);
      const auto g = hex_to_byte(p[2], p[3]);
      const auto b = hex_to_byte(p[4], p[5]);
      if (r > 255 || g > 255 || b > 255) return false;
      if (r < 0 || g < 0 || b < 0) return false;

      color_out.red = r;
      color_out.green = g;
      color_out.blue = b;
      return true;
    }
    if (hex_len == 3) {
      // #RGB → expand to #RRGGBB
      const auto r = hexval(p[0]);
      const auto g = hexval(p[1]);
      const auto b = hexval(p[2]);
      if (r > 255 || g > 255 || b > 255) return false;
      if (r < 0 || g < 0 || b < 0) return false;

      color_out.red = static_cast<uint8_t>((r << 4) | r);
      color_out.green = static_cast<uint8_t>((g << 4) | g);
      color_out.blue = static_cast<uint8_t>((b << 4) | b);
      return true;
    }

    return false;// invalid hex length
  }

  // --- DECIMAL MODE ------------------------------------------------------
  auto parse_int = [&](uint8_t &out) -> bool {
    while (p < end && *p == ' ') p++;

    int value = 0;
    auto res = std::from_chars(p, end, value);

    if (res.ec != std::errc{} || value < 0 || value > 255)
      return false;

    out = static_cast<uint8_t>(value);
    p = res.ptr;
    return true;
  };

  return parse_int(color_out.red) && parse_int(color_out.green) && parse_int(color_out.blue);
}

}// namespace

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
