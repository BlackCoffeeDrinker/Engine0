#include "tests.hpp"

using namespace e00;

class TestResource : public Resource {
public:
  TestResource(Vec2D<uint16_t>, DrawableSurface::BitDepth) {}
  [[nodiscard]] type_t Type() const override { return e00::type_id<TestResource>(); }
  [[nodiscard]] size_t SizeUsage() override { return 0; }
};

class AnEngine : public Engine {
public:
  explicit AnEngine() {
    AddText("en", 1, "Hello World");
  }

  ~AnEngine() override = default;

  [[nodiscard]] std::string_view Name() const noexcept override {
    return "TestEngine";
  }

protected:
  std::error_code OnInit() override {
    return {};
  }

  void OnFirstTick() override {
  }
};


static std::unique_ptr<Engine> CreateGameEngine() {
  auto ptr = std::make_unique<AnEngine>();
  if (ptr->Init()) {
    return nullptr;
  }

  return ptr;
}

/********************************************************************************************/


TEST_CASE("Engine can be created") {
  auto engine = CreateGameEngine();
  REQUIRE(engine != nullptr);
}

TEST_CASE("Engine can return it's name") {
  auto engine = CreateGameEngine();
  REQUIRE(engine != nullptr);
  REQUIRE(engine->Name() == "TestEngine");
}

TEST_CASE("Resource PTR", "[core]") {

  {
    auto res = ResourceManager::GlobalResourceManager().TakeOwnership(
        std::make_unique<TestResource>(e00::Vec2D<uint16_t>(120, 120), DrawableSurface::BitDepth::DEPTH_8));
  }
  {
    auto res = ResourceManager::GlobalResourceManager().TakeOwnership(
        std::make_unique<TestResource>(e00::Vec2D<uint16_t>(120, 120), DrawableSurface::BitDepth::DEPTH_8));
  }
  {
    auto res = ResourceManager::GlobalResourceManager().TakeOwnership(
        std::make_unique<TestResource>(e00::Vec2D<uint16_t>(120, 120), DrawableSurface::BitDepth::DEPTH_8));
  }
  {
    auto res = ResourceManager::GlobalResourceManager().TakeOwnership(
        std::make_unique<TestResource>(e00::Vec2D<uint16_t>(120, 120), DrawableSurface::BitDepth::DEPTH_8));
  }
}
