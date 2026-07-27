#include <Engine.hpp>

namespace {

}// namespace

namespace e00 {
ResourcePtrT<WindowedDrawableResource> WindowedDrawableResource::MakeSubSurface(const ResourcePtrT<DrawableResource> &source, const RectT<BitmapSizeType> &rect) {
  return nullptr;
}

WindowedDrawableResource::WindowedDrawableResource(const ResourcePtrT<DrawableResource> &source,
                                                   const RectT<BitmapSizeType> &rect)
    : DrawableResource(rect.Size(), source->GetBitDepth()),
      _originalId(source.Id()),
      _windowRect(rect) {
}

WindowedDrawableResource::~WindowedDrawableResource() {
}

}// namespace e00
