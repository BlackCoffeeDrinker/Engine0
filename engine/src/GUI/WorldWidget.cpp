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
  const auto &tileset = world.Map()->Tileset();

  // Compute the ## of tiles needed
  const auto worldSizeInTiles = Size() / tileset.TileSize();

  // Adjust the "viewport"; do not go over the map
  const Vec2D adjSize(worldSizeInTiles.Clamp(world.Size()));

  // Compute window
  const Vec2D start = {
    computeStartForCenter(_cameraCenter.x, adjSize.x, world.Width()),
    computeStartForCenter(_cameraCenter.y, adjSize.y, world.Height())
  };

  for (uint16_t y = 0; y < adjSize.y; y++) {
    for (uint16_t x = 0; x < adjSize.x; x++) {
      const auto tileId = world.Map()->Get(start + Vec2D{ x, y });
      const Vec2D tilePos = { x, y };

      painter.drawBitmap(
        { tilePos * tileset.TileSize(), tileset.TileSize() },
        tileset.GetTileRect(tileId),
        tileset.GetBitmap().Ref());
    }
  }
}
void WorldWidget::ResizeEvent() {
  Widget::ResizeEvent();
}

void WorldWidget::ComputeSize() {
  // Expand to take up to maximum size
  if (_worldToDraw) {
  }
}

void WorldWidget::Paint(Painter &painterObj) {
  if (_worldToDraw) {
    DrawWorld(painterObj, *_worldToDraw);
  } else {
    painterObj.setBrushColor({ 0, 0, 0 });
    painterObj.fillRect({ Position(), Size() });
  }
}
}// namespace e00
