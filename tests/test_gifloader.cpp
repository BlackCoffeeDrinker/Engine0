#include "tests.hpp"

#include "Loaders/SpriteGifLoader.hpp"


TEST_CASE("Load Sample Gif", "[resources]") {
  e00::impl::GifSpriteLoader loader;
  REQUIRE(loader.SupportsType(e00::type_id<e00::Sprite>()) == true);

  const auto stream = TestFileStream::CreateFromFilename("tests/sample_1.gif");
  REQUIRE(stream != nullptr);

  REQUIRE(loader.CanLoad(*stream));
  (void) stream->SeekTo(0);

  if (const auto sp = loader.ReadLoad(*stream, e00::type_id<e00::Sprite>());
      sp.IsType<e00::Sprite>()) {
    auto& sprite = sp.resource->As<e00::Sprite>();
    REQUIRE(sprite.Size().x == 10);
    
    sprite.SetCurrentTime(std::chrono::milliseconds(0));
    
    const auto out = TestFileStream::CreateFromFilename("test1.bmp", true);
    sprite.SaveToBMP(*out);
  }
}

TEST_CASE("Load Sample Animated Gif", "[resources]") {
  e00::impl::GifSpriteLoader loader;
  REQUIRE(loader.SupportsType(e00::type_id<e00::Sprite>()) == true);

  const auto stream = TestFileStream::CreateFromFilename("tests/sample-animated-400x300.gif");
  REQUIRE(stream != nullptr);

  REQUIRE(loader.CanLoad(*stream));
  (void) stream->SeekTo(0);

  if (const auto sp = loader.ReadLoad(*stream, e00::type_id<e00::Sprite>());
      sp.IsType<e00::Sprite>()) {
    auto& sprite = sp.resource->As<e00::Sprite>();
    
    sprite.SetCurrentTime(std::chrono::milliseconds(600));
    
    const auto out = TestFileStream::CreateFromFilename("test2.bmp", true);
    sprite.SaveToBMP(*out);
  }
}
