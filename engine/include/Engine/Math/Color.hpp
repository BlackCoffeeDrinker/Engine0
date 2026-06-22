
#pragma once


#include <cstdint>

namespace e00 {
/**
 * RGB color holder
 */
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

  /**
   * Constructs a Color object from a 32-bit RGB value.
   *
   * This method extracts the red, green, and blue channel values from the
   * given 32-bit RGB integer and initializes a Color object with these values.
   *
   * @param rgb A 32-bit integer representing a color in RGB format.
   * @return A Color object with the extracted red, green, and blue channel values.
   */
  static constexpr Color fromRGB(uint32_t rgb) {
    return {
        ExtractColor<3>(rgb),
        ExtractColor<2>(rgb),
        ExtractColor<1>(rgb),
    };
  }

  /**
   * Constructs a Color object from a 32-bit BGR value.
   *
   * This method extracts the blue, green, and red channel values from the
   * given 32-bit BGR integer and initializes a Color object with these values.
   *
   * @param bgr A 32-bit integer representing a color in BGR format.
   * @return A Color object with the extracted blue, green, and red channel values.
   */
  static constexpr Color fromBGR(uint32_t bgr) {
    return {
        ExtractColor<1>(bgr),
        ExtractColor<2>(bgr),
        ExtractColor<3>(bgr),
    };
  }

  constexpr Color() : red(0), green(0), blue(0) {}

  constexpr Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}

  /**
   * Computes the minimum value among the red, green, and blue channel values.
   *
   * The function compares the intensity values of the red, green, and blue channels
   * and returns the smallest value.
   *
   * @return The minimum channel value as an unsigned 8-bit integer.
   */
  [[nodiscard]] uint8_t get_rgb_min() const noexcept {
    return red < green ? (red < blue ? red : blue) : (green < blue ? green : blue);
  }

  /**
   * Computes the maximum value among the red, green, and blue channel values.
   *
   * The function compares the intensity values of the red, green, and blue channels
   * and returns the highest value.
   *
   * @return The maximum channel value as an unsigned 8-bit integer.
   */
  [[nodiscard]] uint8_t get_rgb_max() const noexcept {
    return red > green ? (red > blue ? red : blue) : (green > blue ? green : blue);
  }

  /**
   * Computes the luma (weighted sum of RGB channel values) of the color.
   *
   * This function uses the standard luma formula, where the red, green, and blue
   * channels are weighted by 299, 587, and 114 respectively. The result is computed
   * as an integer without floating-point operations.
   *
   * Luma value, CCIR 601 weighted
   *
   * @return The luma value as an unsigned 32-bit integer.
   */
  [[nodiscard]] uint32_t luma() const noexcept {
    return red * 299u + green * 587u + blue * 114u;
  }

  /**
   * Computes the squared Euclidean distance to another color.
   *
   * Calculates the sum of the squared differences of the red, green, and blue
   * channel values between the current color and the given color.
   *
   * @param other The color to which the distance is computed.
   * @return The squared Euclidean distance as a double.
   */
  [[nodiscard]] uint32_t distanceTo(const Color &other) const noexcept {
    const auto dr = static_cast<int32_t>(red) - static_cast<int32_t>(other.red);
    const auto dg = static_cast<int32_t>(green) - static_cast<int32_t>(other.green);
    const auto db = static_cast<int32_t>(blue) - static_cast<int32_t>(other.blue);

    return static_cast<uint32_t>((dr * dr) + (dg * dg) + (db * db));
  }

  explicit operator uint32_t() const noexcept {
    return static_cast<uint32_t>((red << RED_SHIFT) + (green << GREEN_SHIFT) + blue);
  }

  /**
   * Compares two Color objects for equality.
   *
   * This operator checks if the red, green, and blue channel values of the
   * current color are equal to the corresponding channel values of the given
   * color.
   *
   * @param rhs The Color object to compare with.
   * @return True if the colors are equal in all channels, false otherwise.
   */
  bool operator==(const Color &rhs) const noexcept {
    return (red == rhs.red) && (green == rhs.green) && (blue == rhs.blue);
  }

  /**
   * Compares two Color objects for inequality.
   *
   * This operator checks if the red, green, or blue channel values of the
   * current color are not equal to the corresponding channel values of the given
   * color.
   *
   * @param rhs The Color object to compare with.
   * @return True if the colors differ in at least one channel, false otherwise.
   */
  bool operator!=(const Color &rhs) const noexcept {
    return !(*this == rhs);
  }
};

class ColorOrIndex final {
  bool colorSet = false;
  Color color = {};
  bool indexSet = false;
  uint8_t index = 0;

public:
  ColorOrIndex() = default;
  ColorOrIndex(const Color &color) : colorSet(true), color(color) {}
  ColorOrIndex(uint8_t index) : indexSet(true), index(index) {}
  ColorOrIndex(const Color &color, uint8_t index) : colorSet(true), color(color), indexSet(true), index(index) {}

  [[nodiscard]] bool isColor() const { return colorSet; }
  [[nodiscard]] const Color &getColor() const { return color; }
  [[nodiscard]] bool isIndex() const { return indexSet; }
  [[nodiscard]] uint8_t getIndex() const { return index; }

  void setColor(const Color &color) {
    colorSet = true;
    this->color = color;
  }
  void setIndex(uint8_t index) {
    indexSet = true;
    this->index = index;
  }

  void clearColor() {
    colorSet = false;
  }
  void clearIndex() {
    indexSet = false;
  }
};

}// namespace e00
