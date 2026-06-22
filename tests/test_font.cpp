#include "tests.hpp"

TEST_CASE("Font rendering test", "[font]") {
    auto target = e00::Bitmap::Create({100, 20}, e00::DrawableSurface::BitDepth::DEPTH_32);
    e00::Font &font = e00::Font::DefaultFont();
    
    e00::Color fg(255, 255, 255);
    e00::Color bg(0, 0, 0);
    
    // Render "Hello"
    {
        auto painter = target->BeginDraw();
        font.Render("Hello", fg, bg, *painter, {{0, 0}, {100, 20}});
    }
    
    auto wstream = e00::StreamFactory::GlobalStreamFactory().OpenStreamForWrite("font_test.bmp");
    REQUIRE(wstream != nullptr);
    target->SaveToBMP(*wstream);
    
    // Check if some pixels are drawn
    // We can't easily check the content of the BMP here without more complex logic,
    // but we can at least ensure it doesn't crash and maybe check a few pixels in the bitmap if we had access to them.
    // Bitmap has GetLineData.
    
    bool found_fg = false;
    for (uint16_t y = 0; y < 20; ++y) {
        auto line = target->GetLineData(y);
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] != 0) {
                found_fg = true;
                break;
            }
        }
        if (found_fg) break;
    }
    
    REQUIRE(found_fg == true);
}
