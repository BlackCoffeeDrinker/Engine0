#include "Platform.hpp"
#include <Engine/Platform/Painter.hpp>

namespace platform {
std::unique_ptr<e00::DrawableSurface> Optimize(const e00::DrawableSurface &source, bool preferHW) {
  auto &mainSurface = GetMainSurface();
  std::unique_ptr<e00::DrawableSurface> surf;

  if (preferHW) {
    surf = mainSurface.CreateOptimizedSurface(source.Size(), platform::MemoryPlacement::VideoMemOnly);
  }

  if (!surf) {
    surf = mainSurface.CreateOptimizedSurface(source.Size(), platform::MemoryPlacement::SystemMemory);
  }

  if (surf) {
    if (const auto painter = surf->BeginDraw()) {
      painter->DrawSurface(source, {{0, 0}, source.Size()}, {0, 0});
      return surf;
    }
  }

  return nullptr;
}
}// namespace platform
