#include <Engine.hpp>

namespace e00 {
bool ParseNumber(const std::string_view &input, int &output) {
  std::string_view sv = input;
  if (!sv.empty() && sv.front() == '+') {
    sv.remove_prefix(1);
  }

  auto [ptr, ec] = ParseChars(sv, output);
  return ec == ParseError::ok && ptr == sv.data() + sv.size();
}

bool ParseWorldPoint(const std::string_view &value, WorldPosition &out) {
  const auto comma = value.find(',');
  if (comma == std::string_view::npos) {
    return false;
  }

  const auto xStr = value.substr(0, comma);
  const auto yStr = value.substr(comma + 1);

  int x = 0;
  int y = 0;
  if (!ParseNumber(xStr, x)) {
    return false;
  }
  if (!ParseNumber(yStr, y)) {
    return false;
  }

  out = {static_cast<WorldCoordinateType>(x), static_cast<WorldCoordinateType>(y)};
  return true;
}

bool ParseBool(const std::string_view &value, bool defaultValue) {
  if (value == "true" || value == "1") {
    return true;
  }
  if (value == "false" || value == "0") {
    return false;
  }
  return defaultValue;
}

bool ParseRGB(const std::string_view &sv, Color &color_out) {
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
    auto res = ParseChars(std::string_view(p, end), value);

    if (res.ec != ParseError::ok || value < 0 || value > 255)
      return false;

    out = static_cast<uint8_t>(value);
    p = res.ptr;
    return true;
  };

  return parse_int(color_out.red) && parse_int(color_out.green) && parse_int(color_out.blue);
}
}// namespace e00
