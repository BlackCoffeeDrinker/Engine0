
#pragma once

namespace e00 {

struct Color final {
  enum {
    RED_MASK = 0x00FF0000u,
    GREEN_MASK = 0x0000FF00u,
    BLUE_MASK = 0x000000FFu,

    RED_SHIFT = 16u,
    GREEN_SHIFT = 8u,

    CHANNEL_MASK = 0xFF
  };

  template<uint32_t T>
  constexpr static uint8_t ExtractColor(const uint32_t &t) {
    constexpr uint32_t SHIFT = (T - 1) * 8;
    return static_cast<uint8_t>((t >> SHIFT) & 0xFF);
  }

  uint8_t red;
  uint8_t green;
  uint8_t blue;

  static constexpr Color fromRGB(uint32_t rgb) {
    return {
      ExtractColor<3>(rgb),
      ExtractColor<2>(rgb),
      ExtractColor<1>(rgb),
    };
  }

  static constexpr Color fromBGR(uint32_t bgr) {
    return {
      ExtractColor<1>(bgr),
      ExtractColor<2>(bgr),
      ExtractColor<3>(bgr),
    };
  }

  constexpr Color() : red(0), green(0), blue(0) {}

  constexpr Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

  [[nodiscard]] uint8_t get_rgb_min() const noexcept {
    return red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
  }

  [[nodiscard]] uint8_t get_rgb_max() const noexcept {
    return red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
  }

  /**
   * Luma value, CCIR 601 weighted
   *
   * @return 0 to 255000
   */
  [[nodiscard]] uint32_t luma() const noexcept {
    return red * 299u + green * 587u + blue * 114u;
  }

  explicit operator uint32_t() const noexcept {
    return static_cast<uint32_t>((red << RED_SHIFT) + (green << GREEN_SHIFT) + blue);
  }

  bool operator==(const Color &rhs) const noexcept {
    return (red == rhs.red)
           && (green == rhs.green)
           && (blue == rhs.blue);
  }
};
}// namespace e00