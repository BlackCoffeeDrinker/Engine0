#include <Engine.hpp>

namespace {
template<typename T>
constexpr T computeStartForCenter(const T &center, const T &wanted, const T &max) {
  const auto half = wanted / 2;
  if ((half + center) > max) {
    return max - wanted;
  }

  if (half > center) {
    return 0;
  }

  return center - half;
}
}// namespace

namespace e00 {
WorldWidget::WorldWidget(const std::unique_ptr<World> &worldToDraw) : _worldToDraw(worldToDraw) {
}

void WorldWidget::DrawWorld(Painter &painter, const World &world) {
  const auto tile_size = world.TileSize();

  // Compute the ## of tiles needed
  const auto worldSizeInTiles = Size() / tile_size;

  // Adjust the "viewport"; do not go over the map
  const Vec2D adjSize(worldSizeInTiles.Clamp(world.Size()));

  // Compute window
  const Vec2D start = {
      computeStartForCenter(_cameraCenter.x, adjSize.x, world.Width()),
      computeStartForCenter(_cameraCenter.y, adjSize.y, world.Height())};

  world.PaintMap(start, start + adjSize, painter, {0, 0});

  // Draw actors
  const RectT viewport(start, adjSize);
  for (const auto &element: world.Actors()) {
    if (element.actor != nullptr && viewport.Contains(element.position)) {
      const auto relativePosition = element.position - start;
      const auto drawPosition = AbsolutePosition() + relativePosition * tile_size;
      element.actor->Draw(painter, drawPosition);
    }
  }
}

void WorldWidget::ResizeEvent() {
  Widget::ResizeEvent();
  // TODO: Cache worldSizeInTiles
  _world_bitmap = nullptr;
}


void WorldWidget::Paint(Painter &painterObj) {
  if (_worldToDraw) {
    if (!_world_bitmap) {
      const auto targetInfo = painterObj.GetTargetInformation();
      _world_bitmap = Bitmap::Create(Size(), targetInfo.bit_depth, targetInfo.palette ? *targetInfo.palette : FixedPalette());
    }

    // Draw everything to a temp bitmap to blitz to the final screen
    if (const auto tmpPainter = _world_bitmap->BeginDraw()) {
      DrawWorld(*tmpPainter, *_worldToDraw);
    }
    
    painterObj.BlitSurface(
        *_world_bitmap,
        {{0, 0}, _world_bitmap->Size()},
        AbsolutePosition());

  } else {
    painterObj.SetBrushColor({0, 0, 0});
    painterObj.SetNoPen();
    painterObj.DrawRect(AbsoluteComputedRect());
  }
}
}// namespace e00
