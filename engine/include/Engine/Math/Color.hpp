
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
  [[nodiscard]] double distanceTo(const Color &other) const {
    return std::pow(red - other.red, 2)
           + std::pow(green - other.green, 2)
           + std::pow(blue - other.blue, 2);
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
    return (red == rhs.red)
           && (green == rhs.green)
           && (blue == rhs.blue);
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

/**
 * Class representing a palette with a fixed numbers of colors.
 *
 */
struct FixedPalette final {
  std::array<Color, 256> colors;
  uint8_t numberOfColors;

  constexpr FixedPalette() : numberOfColors(0) {}

  constexpr explicit FixedPalette(const size_t numberOfColors) : numberOfColors(numberOfColors) {
    if (numberOfColors > colors.size()) {
      abort();
    }
  }

  ~FixedPalette() = default;

  constexpr Color operator[](const size_t index) const noexcept {
    if (index >= numberOfColors) {
      abort();
      return {};
    }
    return colors.at(index);
  }

  Color &operator[](const size_t index) noexcept {
    if (index >= numberOfColors) {
      abort();
      return colors.at(0);
    }
    return colors.at(index);
  }

  /**
   * Compares two FixedPalette objects to determine if they contain the same set of colors.
   *
   * This function checks if the two palettes have the same number of colors and whether
   * the colors at each respective index are identical. The comparison stops as soon as a
   * mismatch is found.
   *
   * @param rhs The FixedPalette object to compare against.
   * @return True if both palettes have the same number of colors and identical colors at
   *         each corresponding position; false otherwise.
   */
  [[nodiscard]] bool isSamePalette(const FixedPalette &rhs) const noexcept {
    const bool sameSize = numberOfColors == rhs.numberOfColors;
    return sameSize
           && std::equal(
             colors.begin(), colors.begin() + numberOfColors, rhs.colors.begin());
  }

  /**
   * Identifies the index of the closest matching color in a fixed palette.
   *
   * This function iterates over the available colors in the palette to find
   * the color that is either an exact match to the input color or has the smallest
   * squared Euclidean distance to it. If an exact match is found, its index is
   * immediately returned. Otherwise, the index of the color with the smallest
   * distance is returned.
   *
   * @param x The reference color to find the closest match for.
   * @return The index of the closest matching color (exact or minimum distance).
   */
  [[nodiscard]] uint8_t findClosestColorIndex(const Color &x) const {
    uint8_t closestIndex = 0;
    double minDistance = std::numeric_limits<double>::max();

    for (uint8_t i = 0; i < numberOfColors; ++i) {
      const auto &value = colors.at(i);

      // Do we have an exact match?
      if (value == x) {
        return i;
      }

      // Compute distance and check if it's lower then the one we have
      if (const auto distance = value.distanceTo(x); distance < minDistance) {
        minDistance = distance;
        closestIndex = i;
      }
    }

    return closestIndex;
  }

  [[nodiscard]] auto size() const noexcept {
    return numberOfColors;
  }
  
  [[nodiscard]] auto begin() const noexcept {
    return colors.begin();
  }
  
  [[nodiscard]] auto end() const noexcept {
    return colors.begin() + numberOfColors;
  }
  /**
   * Changes the number of colors in this palette
   *
   * @param num_colors_in_palette The new number of colors in this palette
   */
  void resize(const int num_colors_in_palette) {
    if (num_colors_in_palette > colors.size()) {
      abort();
    }
    numberOfColors = num_colors_in_palette;
  }

  /**
   * Indicates whether the FixedPalette contains any colors.
   *
   * @return True if the palette contains one or more colors; false otherwise.
   */
  explicit operator bool() const noexcept {
    return numberOfColors > 0;
  }

  bool operator==(const FixedPalette &rhs) const noexcept {
    return isSamePalette(rhs);
  }

  bool operator!=(const FixedPalette &rhs) const noexcept {
    return !(*this == rhs);
  }

  FixedPalette &operator=(const FixedPalette &rhs) {
    numberOfColors = rhs.numberOfColors;
    std::copy(rhs.colors.begin(), rhs.colors.end(), colors.begin());
    return *this;
  }
};

}// namespace e00