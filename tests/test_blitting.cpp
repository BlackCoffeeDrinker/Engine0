#include <catch2/catch_all.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Platform/DrawableSurface.hpp>
#include <Engine/DefaultBitmapHelpers.hpp>

using namespace e00;

TEST_CASE("Bitmap Blitting - Bit Depth Conversion", "[blitting]") {
    SECTION("8-bit to 32-bit conversion") {
        auto src = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
        (void)src->SetPaletteColor(1, Color(255, 0, 0)); // Red
        
        auto srcData = src->GetLineData(0);
        srcData[0] = 1; // Set first pixel to index 1 (red)

        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_32);
        {
            auto painter = dst->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {10, 10}}, {0, 0});
        }

        DrawableSurface::TargetInformation info32;
        info32.bit_depth = DrawableSurface::BitDepth::DEPTH_32;
        info32.shift = {16, 8, 0}; // R, G, B shifts
        info32.mask = {0xFF, 0xFF, 0xFF};
        
        std::vector<uint8_t> result(40);
        dst->ReadLineInto(0, 0, 10, info32, result);
        
        // Pixel 0 should be red
        CHECK(result[0] == 0);   // Blue
        CHECK(result[1] == 0);   // Green
        CHECK(result[2] == 255); // Red
        
        // Pixel 1 should be black (0)
        CHECK(result[4] == 0);
        CHECK(result[5] == 0);
        CHECK(result[6] == 0);
    }

    SECTION("32-bit to 8-bit conversion") {
        auto src = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_32);
        {
            auto painter = src->BeginDraw();
            painter->SetPenSolid(1, Color(0, 255, 0)); // Green
            painter->DrawPoint({0, 0});
        }

        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
        (void)dst->SetPaletteColor(1, Color(0, 255, 0)); // Green at index 1
        
        {
            auto painter = dst->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {10, 10}}, {0, 0});
        }

        auto dstData = dst->GetLineData(0);
        CHECK(dstData[0] == 1);
    }
    
    SECTION("1-bit to 32-bit conversion") {
        auto src = Bitmap::Create({16, 1}, DrawableSurface::BitDepth::DEPTH_1, 2);
        (void)src->SetPaletteColor(0, Color(0, 0, 0));
        (void)src->SetPaletteColor(1, Color(255, 255, 255));
        
        auto srcData = src->GetLineData(0);
        srcData[0] = 0x80; // 1000 0000 in binary -> first pixel is index 1 (white)
        
        auto dst = Bitmap::Create({16, 1}, DrawableSurface::BitDepth::DEPTH_32);
        {
            auto painter = dst->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {16, 1}}, {0, 0});
        }
        
        DrawableSurface::TargetInformation info32;
        info32.bit_depth = DrawableSurface::BitDepth::DEPTH_32;
        info32.shift = {16, 8, 0};
        info32.mask = {0xFF, 0xFF, 0xFF};
        
        std::vector<uint8_t> result(16 * 4);
        dst->ReadLineInto(0, 0, 16, info32, result);
        
        // Pixel 0 should be white
        CHECK(result[0] == 255);
        CHECK(result[1] == 255);
        CHECK(result[2] == 255);
        
        // Pixel 1 should be black
        CHECK(result[4] == 0);
        CHECK(result[5] == 0);
        CHECK(result[6] == 0);
    }

    SECTION("8-bit to 8-bit conversion with different palettes") {
        auto src = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 2);
        (void)src->SetPaletteColor(0, Color(0, 0, 0));
        (void)src->SetPaletteColor(1, Color(255, 0, 0)); // Red at index 1
        
        auto srcData = src->GetLineData(0);
        srcData[0] = 1;

        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 2);
        (void)dst->SetPaletteColor(0, Color(0, 0, 0));
        (void)dst->SetPaletteColor(1, Color(255, 0, 0)); // Red at index 1
        
        {
            auto painter = dst->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {10, 10}}, {0, 0});
        }
        
        auto dstData = dst->GetLineData(0);
        CHECK(dstData[0] == 1);

        // Test where red is at different index
        auto dst3 = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 3);
        (void)dst3->SetPaletteColor(0, Color(0, 0, 0));
        (void)dst3->SetPaletteColor(1, Color(0, 0, 0));
        (void)dst3->SetPaletteColor(2, Color(255, 0, 0)); // Red at index 2
        
        {
            auto painter = dst3->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {10, 10}}, {0, 0});
        }
        
        auto dstData3 = dst3->GetLineData(0);
        CHECK(dstData3[0] == 2);
    }

    SECTION("1-bit to 8-bit conversion") {
        auto src = Bitmap::Create({8, 1}, DrawableSurface::BitDepth::DEPTH_1, 2);
        (void)src->SetPaletteColor(0, Color(0, 0, 0));
        (void)src->SetPaletteColor(1, Color(0, 255, 0)); // Green
        
        auto srcData = src->GetLineData(0);
        srcData[0] = 0xAA; // 1010 1010 -> pixels 0, 2, 4, 6 are index 1 (green)
        
        auto dst = Bitmap::Create({8, 1}, DrawableSurface::BitDepth::DEPTH_8, 2);
        (void)dst->SetPaletteColor(0, Color(0, 0, 0));
        (void)dst->SetPaletteColor(1, Color(0, 255, 0)); // Green
        
        {
            auto painter = dst->BeginDraw();
            painter->DrawSurface(*src, {{0, 0}, {8, 1}}, {0, 0});
        }
        
        auto dstData = dst->GetLineData(0);
        CHECK(dstData[0] == 1);
        CHECK(dstData[1] == 0);
        CHECK(dstData[2] == 1);
        CHECK(dstData[3] == 0);
        CHECK(dstData[4] == 1);
        CHECK(dstData[5] == 0);
        CHECK(dstData[6] == 1);
        CHECK(dstData[7] == 0);
    }
}
