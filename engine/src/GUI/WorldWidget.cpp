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

  for (WorldCoordinateType y = 0; y < adjSize.y; y++) {
    for (WorldCoordinateType x = 0; x < adjSize.x; x++) {
      const Vec2D tilePos = {x, y};
      const auto drawPosition = AbsolutePosition() + tilePos * tile_size;

      world.PaintTile(start + tilePos, painter, drawPosition);
    }
  }
}

void WorldWidget::ResizeEvent() {
  Widget::ResizeEvent();
  // TODO: Cache worldSizeInTiles
}


void WorldWidget::Paint(Painter &painterObj) {
  if (_worldToDraw) {
    DrawWorld(painterObj, *_worldToDraw);
  } else {
    painterObj.SetBrushColor({0, 0, 0});
    painterObj.SetNoPen();
    painterObj.DrawRect(AbsoluteComputedRect());
  }
}
}// namespace e00
