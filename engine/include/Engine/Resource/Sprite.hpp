#pragma once

#include <Engine/Resource/DrawableResource.hpp>

namespace e00 {
/**
 * An animated bitmap with a mask
 */
class Sprite : public DrawableResource {
  struct Image;

  FixedPalette _palette;

  std::chrono::milliseconds _current_time;
  std::chrono::milliseconds _total_time;
  bool _loops;

  std::array<std::unique_ptr<Image>, 100> _images;
  decltype(_images)::iterator _current_image;

  [[nodiscard]] Image *GetCurrentImage() const { return _current_image != _images.end() ? _current_image->get() : nullptr; }

  Sprite(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette);
  Sprite(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette = 0);

public:
  static std::unique_ptr<Sprite> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, FixedPalette palette);
  static std::unique_ptr<Sprite> Create(const Vec2D<BitmapSizeType> &size, BitDepth bit_depth, int numColorsInPalette = 0);

  Sprite(const Sprite &other) = delete;
  ~Sprite() override;

  /**
   * Sets whether the animation loops when its current time exceeds the total duration.
   */
  void SetLoops(bool loops) { _loops = loops; }

  /**
   * Sets the animation playback time and updates the current frame accordingly.
   *
   * If looping is enabled, the time wraps around the total animation duration.
   * If looping is disabled, the time is clamped to the end of the animation.
   */
  void SetCurrentTime(std::chrono::milliseconds time);

  /**
   * Selects the current frame by index.
   *
   * If looping is enabled, indexes beyond the frame count wrap around.
   * If looping is disabled, out-of-range indexes are ignored.
   */
  void SetImageIndex(size_t index);

  [[nodiscard]] bool Loops() const { return _loops; }
  [[nodiscard]] auto NumberOfImages() const { return _images.size(); }
  [[nodiscard]] auto CurrentTime() const { return _current_time; }

  [[nodiscard]] type_t Type() const override { return type_id<Sprite>(); }
  [[nodiscard]] size_t GetNumberOfColorsInPalette() const override { return _palette.size(); }
  [[nodiscard]] Color GetColorFromPalette(size_t index) const override {
    if (index < _palette.size()) {
      return _palette[index];
    }
    return {};
  }


  [[nodiscard]] std::unique_ptr<Painter> BeginDraw() override;

  void ReadLineInto(
      BitmapSizeType line,
      BitmapSizeType startX, BitmapSizeType endX,
      const TargetInformation &targetInformation, std::span<uint8_t> targetBuffer) const override;

  /**
   * Adds a frame to the end of the frame list
   * 
   * @param data The bitmap data to use
   * @param duration the duration this frame should be shown
   * @return any errors (if the bitmap is different depth as the rest, ...)
   */
  std::error_code AddFrame(ResourcePtrT<Bitmap> data, std::chrono::milliseconds duration);
  std::error_code AddFrame(std::unique_ptr<Bitmap> &&data, std::chrono::milliseconds duration);
};
}// namespace e00
