#pragma once

namespace e00 {
class World;
class WorldWidget : public Widget {
  const std::unique_ptr<World> &_worldToDraw;
  Vec2D<uint16_t> _cameraCenter;

  void DrawWorld(Painter &painter, const World &world);

protected:
  void ResizeEvent() override;

  void ComputeSize() override;
  
public:
  explicit WorldWidget(const std::unique_ptr<World> &worldToDraw);
  ~WorldWidget() override = default;

  [[nodiscard]] const Vec2D<uint16_t> &CameraCenter() const { return _cameraCenter; }
  void SetCameraCenter(const Vec2D<uint16_t> &camera_center) { _cameraCenter = camera_center; }

  void Paint(Painter &painterObj) override;
};

}// namespace e00
