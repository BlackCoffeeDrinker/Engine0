#include <catch2/catch_test_macros.hpp>

#include <Engine/Detail/StringFormat.hpp>
#include <Engine/Detail/TypeId.hpp>
#include <Engine/Logging/Logger.hpp>
#include <span>
#include <Engine/Platform/ResourceManager.hpp>
#include <Engine/Platform/Stream.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/World.hpp>

#include "Actors/ActorChest.hpp"

#include <cstring>

using namespace e00;

namespace {
class MemoryStream : public Stream {
  std::vector<uint8_t> _data;

protected:
  std::error_code real_read(size_t size, void *data) override {
    std::memcpy(data, _data.data() + Position(), size);
    return {};
  }

  std::error_code real_seek(size_t) override { return {}; }

public:
  explicit MemoryStream(std::vector<uint8_t> data) : Stream(data.size()), _data(std::move(data)) {}
};

class MemoryWritableStream : public WritableStream {
public:
  std::vector<uint8_t> data;

  MemoryWritableStream() : WritableStream(0) {}

protected:
  std::error_code real_write(size_t size, const void *ptr) override {
    const auto *bytes = static_cast<const uint8_t *>(ptr);
    data.insert(data.end(), bytes, bytes + size);
    return {};
  }

  std::error_code real_read(size_t, void *) override { return std::make_error_code(std::errc::function_not_supported); }
  std::error_code real_seek(size_t) override { return {}; }
};

// A single-frame sprite whose only pixel is `markerColor`, so painting it can be verified
// by reading back that pixel's raw index.
ResourcePtrT<Sprite> MakeTestSprite(uint8_t markerColor) {
  auto sprite = Sprite::Create(Vec2D<BitmapSizeType>{1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  auto frame = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  {
    auto painter = frame->BeginDraw();
    painter->SetPenSolid(1, markerColor);
    painter->DrawPoint({0, 0});
  }
  REQUIRE_FALSE(sprite->AddFrame(std::move(frame), std::chrono::milliseconds(100)));
  return ResourceManager::GlobalResourceManager().TakeOwnership(std::move(sprite));
}

uint8_t ReadMarkerPixel(const Bitmap &bmp) {
  std::vector<uint8_t> buf(1);
  const DrawableSurface::TargetInformation info{DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, nullptr, {}, {}};
  bmp.ReadLineInto(0, 0, 1, info, buf);
  return buf[0];
}
}// namespace

TEST_CASE("ActorChest starts closed", "[actor][chest]") {
  impl::ActorChest chest;
  REQUIRE_FALSE(chest.IsOpen());
  REQUIRE(chest.CurrentState().stateId == impl::ActorChest::kClosedStateId);
}

TEST_CASE("ActorChest::OnInteract toggles open/closed and reflects it in the current state", "[actor][chest]") {
  World world("chest-world", {100, 100});
  impl::ActorChest chest;
  const auto node = world.Insert("chest", &chest, {1, 1});
  REQUIRE(node != World::InvalidNodeID);

  REQUIRE(world.Interact(node));
  REQUIRE(chest.IsOpen());
  REQUIRE(chest.CurrentState().stateId == impl::ActorChest::kOpenStateId);

  REQUIRE(world.Interact(node));
  REQUIRE_FALSE(chest.IsOpen());
  REQUIRE(chest.CurrentState().stateId == impl::ActorChest::kClosedStateId);
}

TEST_CASE("ActorChest save/load round trip preserves open state", "[actor][chest]") {
  impl::ActorChest openChest;
  REQUIRE(openChest.HasSavableState());

  World world("chest-world", {100, 100});
  const auto node = world.Insert("chest", &openChest, {1, 1});
  REQUIRE(world.Interact(node));
  REQUIRE(openChest.IsOpen());

  MemoryWritableStream out;
  REQUIRE(openChest.SaveState(out));

  impl::ActorChest reloadedChest;
  REQUIRE_FALSE(reloadedChest.IsOpen());

  MemoryStream in(out.data);
  REQUIRE(reloadedChest.LoadState(in));
  REQUIRE(reloadedChest.IsOpen());
  REQUIRE(reloadedChest.CurrentState().stateId == impl::ActorChest::kOpenStateId);
}

TEST_CASE("ActorChest::Draw paints the closed sprite by default and the open sprite after interacting", "[actor][chest][draw]") {
  impl::ActorChest chest;
  chest.SetClosedSprite(MakeTestSprite(11));
  chest.SetOpenSprite(MakeTestSprite(22));

  auto canvas = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);

  {
    auto painter = canvas->BeginDraw();
    chest.Draw(*painter, {0, 0}, Direction::South, std::chrono::milliseconds(0));
  }
  REQUIRE(ReadMarkerPixel(*canvas) == 11);

  World world("chest-draw-world", {100, 100});
  const auto node = world.Insert("chest", &chest, {1, 1});
  REQUIRE(node != World::InvalidNodeID);
  REQUIRE(world.Interact(node));
  REQUIRE(chest.IsOpen());

  {
    auto painter = canvas->BeginDraw();
    chest.Draw(*painter, {0, 0}, Direction::South, std::chrono::milliseconds(0));
  }
  REQUIRE(ReadMarkerPixel(*canvas) == 22);
}

TEST_CASE("ActorChest::Draw with no sprite set is a graceful no-op", "[actor][chest][draw]") {
  impl::ActorChest chest;
  auto canvas = Bitmap::Create({1, 1}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
  auto painter = canvas->BeginDraw();
  REQUIRE_NOTHROW(chest.Draw(*painter, {0, 0}, Direction::South, std::chrono::milliseconds(0)));
}
