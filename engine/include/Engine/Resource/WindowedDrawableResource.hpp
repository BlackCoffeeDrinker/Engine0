
#pragma once

#include <Engine/Math/Rect.hpp>
#include <Engine/Resource/DrawableResource.hpp>
#include <Engine/ResourcePtr.hpp>

namespace e00 {

/**
 * A part of a DrawableResource
 */
class WindowedDrawableResource : public DrawableResource {
  ResourceId _originalId;           //< resource id of our original source
  RectT<BitmapSizeType> _windowRect;//< the rect in the original source

protected:
  WindowedDrawableResource(
      const ResourcePtrT<DrawableResource> &source,
      const RectT<BitmapSizeType> &rect);

public:
  static ResourcePtrT<WindowedDrawableResource> MakeSubSurface(
      const ResourcePtrT<DrawableResource> &source,
      const RectT<BitmapSizeType> &rect);

  ~WindowedDrawableResource() override;

  [[nodiscard]] ResourceId OriginalId() const { return _originalId; }
  [[nodiscard]] RectT<BitmapSizeType> WindowRect() const { return _windowRect; }
};

}// namespace e00
