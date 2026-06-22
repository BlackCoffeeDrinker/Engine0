#include "tests.hpp"

using namespace e00;

class TestResource : public e00::Resource {
public:
  TestResource(e00::Vec2D<uint16_t>, e00::DrawableSurface::BitDepth) {}
  [[nodiscard]] e00::type_t Type() const override { return e00::type_id<TestResource>(); }
};

class AnEngine : public e00::Engine {
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


static std::unique_ptr<e00::Engine> CreateGameEngine() {
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
    auto res = e00::ResourceManager::GlobalResourceManager().Make<TestResource>("Test Bitmap 1"_id, e00::Vec2D<uint16_t>(120, 120), e00::DrawableSurface::BitDepth::DEPTH_8);
  }
  {
    auto res = e00::ResourceManager::GlobalResourceManager().Make<TestResource>("Test Bitmap 2"_id, e00::Vec2D<uint16_t>(120, 120), e00::DrawableSurface::BitDepth::DEPTH_8);
  }
  {
    auto res = e00::ResourceManager::GlobalResourceManager().Make<TestResource>("Test Bitmap 3"_id, e00::Vec2D<uint16_t>(120, 120), e00::DrawableSurface::BitDepth::DEPTH_8);
  }
  {
    auto res = e00::ResourceManager::GlobalResourceManager().Make<TestResource>("Test Bitmap 4"_id, e00::Vec2D<uint16_t>(120, 120), e00::DrawableSurface::BitDepth::DEPTH_8);
  }
}
