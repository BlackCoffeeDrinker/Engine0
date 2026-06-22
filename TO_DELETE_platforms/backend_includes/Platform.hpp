
#pragma once

#include <Engine.hpp>

namespace platform {
/**
 * 
 */
class Surface : public e00::DrawableSurface {
public:
  [[nodiscard]] virtual std::unique_ptr<e00::Bitmap> ConvertToBitmap() const = 0;
  [[nodiscard]] virtual std::error_code ConvertFromBitmap(const e00::Bitmap &bitmap) = 0;

  /**
   * Copy a bitmap into this surface
   * 
   * @param src the source of the copy
   * @param srcRect the source rectangle in src
   * @param dstPos where to put src(srcRect) in this surface
   */
  virtual void BlitFrom(const DrawableSurface &src, e00::RectT<e00::BitmapSizeType> srcRect, e00::Vec2D<e00::BitmapSizeType> dstPos) = 0;
};

std::string_view PlatformName();

void SetSettings(std::string_view key,
                 std::string_view value);

std::error_code Init();
void Exit();

void Yield();
void SetWindowTitle(const std::string_view &windowTitle);
bool HasFocus();
void ProcessEvents(e00::Engine &engine);
void ProcessDraw(e00::Engine &engine);

std::unique_ptr<e00::LoggerSink> CreateSink(const std::string &name);

const std::unique_ptr<Surface> &GetMainSurface();

// Open a raw file on disk
std::unique_ptr<e00::Stream> OpenStream(const std::string_view &name);
std::unique_ptr<e00::WritableStream> OpenStreamForWrite(const std::string_view &name);

// Make a hardware surface
std::unique_ptr<Surface> CreateSurface(const e00::Vec2D<uint16_t> &size, e00::DrawableSurface::BitDepth depth);

}// namespace platform
