
#pragma once
#include "Engine/Math/Color.hpp"
#include "Engine/Resource.hpp"
#include <algorithm>
#include <array>
#include <limits>

namespace e00 {

/**
 * Class representing a palette with a fixed numbers of colors.
 *
 */
class FixedPalette final : public Resource {
public:
  static constexpr size_t MAX_SIZE = 256;

private:
  std::array<Color, MAX_SIZE> colors;
  size_t numberOfColors;
  bool hasTransparency;
  uint8_t transparencyIndex;

public:
  constexpr FixedPalette()
      : numberOfColors(0),
        hasTransparency(false),
        transparencyIndex(0) {}

  constexpr explicit FixedPalette(const size_t numberOfColors)
      : numberOfColors(numberOfColors),
        hasTransparency(false),
        transparencyIndex(0) {
    if (numberOfColors > colors.size()) {
      abort();
    }
  }

  constexpr FixedPalette(const FixedPalette &other)
      : numberOfColors(other.numberOfColors),
        hasTransparency(other.hasTransparency),
        transparencyIndex(other.transparencyIndex) {
    colors = other.colors;
  }

  [[nodiscard]] type_t Type() const override { return type_id<FixedPalette>(); }

  FixedPalette(FixedPalette &&other) noexcept
      : colors(other.colors),
        numberOfColors(other.numberOfColors),
        hasTransparency(other.hasTransparency),
        transparencyIndex(other.transparencyIndex) {
  }

  ~FixedPalette() override = default;

  constexpr const Color &operator[](const size_t index) const noexcept {
    if (index >= numberOfColors) {
      abort();
    }
    return colors[index];
  }

  Color &operator[](const size_t index) noexcept {
    if (index >= numberOfColors) {
      abort();
    }
    return colors[index];
  }

  void set(const size_t index, const Color &color) noexcept {
    if (index >= numberOfColors) {
      abort();
    }
    colors[index] = color;
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
    return sameSize && std::equal(
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
    uint32_t minDistance = std::numeric_limits<uint32_t>::max();

    const auto r = x.red;
    const auto g = x.green;
    const auto b = x.blue;

    const Color *data = colors.data();
    for (size_t i = 0; i < numberOfColors; ++i) {
      const auto &value = data[i];

      const int32_t dr = static_cast<int32_t>(value.red) - r;
      const int32_t dg = static_cast<int32_t>(value.green) - g;
      const int32_t db = static_cast<int32_t>(value.blue) - b;

      const auto distance = static_cast<uint32_t>(dr * dr + dg * dg + db * db);
      if (distance == 0) {
        return static_cast<uint8_t>(i);
      }

      if (distance < minDistance) {
        minDistance = distance;
        closestIndex = static_cast<uint8_t>(i);
      }
    }

    return closestIndex;
  }

  [[nodiscard]] auto resolveIndex(const ColorOrIndex &colorOrIndex) const {
    if (colorOrIndex.isColor()) {
      return findClosestColorIndex(colorOrIndex.getColor());
    }
    return colorOrIndex.getIndex();
  }

  [[nodiscard]] auto resolveColor(const ColorOrIndex &colorOrIndex) const {
    if (colorOrIndex.isColor()) {
      return colorOrIndex.getColor();
    }
    return colors[colorOrIndex.getIndex()];
  }

  [[nodiscard]] ColorOrIndex get(uint8_t index) const { return {colors[index], index}; }

  [[nodiscard]] auto size() const noexcept { return numberOfColors; }
  [[nodiscard]] auto empty() const noexcept { return numberOfColors == 0; }

  [[nodiscard]] auto begin() const noexcept { return colors.begin(); }
  [[nodiscard]] auto begin() noexcept { return colors.begin(); }
  [[nodiscard]] auto end() const noexcept { return colors.begin() + size(); }
  [[nodiscard]] auto end() noexcept { return colors.begin() + size(); }
  /**
   * Changes the number of colors in this palette
   *
   * @param num_colors_in_palette The new number of colors in this palette
   */
  void resize(const size_t num_colors_in_palette) {
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

  constexpr FixedPalette &operator=(const FixedPalette &rhs) {
    if (this == &rhs) {
      return *this;
    }

    numberOfColors = rhs.numberOfColors;
    hasTransparency = rhs.hasTransparency;
    transparencyIndex = rhs.transparencyIndex;

    for (size_t i = 0; i < numberOfColors; ++i) {
      colors[i] = rhs.colors[i];
    }

    return *this;
  }
};

}// namespace e00
