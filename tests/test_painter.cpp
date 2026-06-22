#include <catch2/catch_all.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Platform/DrawableSurface.hpp>
#include <Engine/DefaultBitmapHelpers.hpp>

using namespace e00;

TEST_CASE("Painter - Basic Drawing", "[painter]") {
    auto bmp = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
    (void)bmp->SetPaletteColor(1, Color(255, 0, 0)); // Red
    (void)bmp->SetPaletteColor(2, Color(0, 255, 0)); // Green

    SECTION("DrawPoint with Index") {
        {
            auto painter = bmp->BeginDraw();
            painter->SetPenSolid(1, 1); // Index 1
            painter->DrawPoint({1, 1});
            painter->SetPenSolid(1, 2); // Index 2
            painter->DrawPoint({2, 2});
        }
        auto data1 = bmp->GetLineData(1);
        CHECK(data1[1] == 1);
        auto data2 = bmp->GetLineData(2);
        CHECK(data2[2] == 2);
    }

    SECTION("DrawPoint with Color") {
        {
            auto painter = bmp->BeginDraw();
            painter->SetPenSolid(1, Color(255, 0, 0)); // Red -> Index 1
            painter->DrawPoint({3, 3});
        }
        auto data = bmp->GetLineData(3);
        CHECK(data[3] == 1);
    }

    SECTION("DrawPoint 16-bit") {
        auto bmp16 = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_16);
        {
            auto painter = bmp16->BeginDraw();
            painter->SetPenSolid(1, Color(255, 0, 0));
            painter->DrawPoint({1, 1});
        }
        
        DrawableSurface::TargetInformation info16;
        info16.bit_depth = DrawableSurface::BitDepth::DEPTH_16;
        info16.shift = {11, 5, 0}; // 5-6-5 RGB
        info16.mask = {0x1F, 0x3F, 0x1F};
        
        std::vector<uint8_t> result(20);
        bmp16->ReadLineInto(1, 0, 10, info16, result);
        // Pixel (1,1) is at result[2,3] if 2 bytes per pixel
        // Red is 255 -> 0x1F in 5 bits.
        // 0x1F shifted by 11 is 0xF800.
        // In little endian: 0x00, 0xF8.
        CHECK(result[2] == 0x00);
        CHECK(result[3] == 0xF8);
    }

    SECTION("DrawPoints") {
        {
            auto painter = bmp->BeginDraw();
            painter->SetPenSolid(1, 1);
            painter->DrawPoints({{0, 0}, {1, 0}, {2, 0}});
        }
        auto data = bmp->GetLineData(0);
        CHECK(data[0] == 1);
        CHECK(data[1] == 1);
        CHECK(data[2] == 1);
    }

    SECTION("DrawLine") {
        {
            auto painter = bmp->BeginDraw();
            painter->SetPenSolid(1, 1);
            painter->DrawLine({0, 0}, {5, 0}); // Horizontal
            painter->DrawLine({0, 1}, {0, 5}); // Vertical
            painter->DrawLine({1, 1}, {3, 3}); // Diagonal
        }
        // Horizontal line
        auto data0 = bmp->GetLineData(0);
        for (int x = 0; x <= 5; ++x) CHECK(data0[x] == 1);
        
        // Vertical line
        for (int y = 1; y <= 5; ++y) {
            auto data = bmp->GetLineData(y);
            CHECK(data[0] == 1);
        }

        // Diagonal line
        for (int i = 1; i <= 3; ++i) {
            auto data = bmp->GetLineData(i);
            CHECK(data[i] == 1);
        }
    }

    SECTION("DrawRect") {
        SECTION("Outline only") {
            {
                auto painter = bmp->BeginDraw();
                painter->SetPenSolid(1, 1);
                painter->SetNoBrush();
                painter->DrawRect({{1, 1}, {4, 4}});
            }
            // Top and bottom edges
            auto data1 = bmp->GetLineData(1);
            auto data4 = bmp->GetLineData(4);
            for (int x = 1; x <= 4; ++x) {
                CHECK(data1[x] == 1);
                CHECK(data4[x] == 1);
            }
            // Left and right edges
            for (int y = 1; y <= 4; ++y) {
                auto data = bmp->GetLineData(y);
                CHECK(data[1] == 1);
                CHECK(data[4] == 1);
            }
            // Center should be empty (0)
            auto data2 = bmp->GetLineData(2);
            CHECK(data2[2] == 0);
        }

        SECTION("Fill only") {
            {
                auto painter = bmp->BeginDraw();
                painter->SetNoPen();
                painter->SetBrushIndex(2);
                painter->DrawRect({{1, 1}, {3, 3}});
            }
            for (int y = 1; y <= 3; ++y) {
                auto data = bmp->GetLineData(y);
                for (int x = 1; x <= 3; ++x) {
                    CHECK(data[x] == 2);
                }
            }
            // Border should not be touched outside the rect
            auto data0 = bmp->GetLineData(0);
            CHECK(data0[0] == 0);
            auto data4 = bmp->GetLineData(4);
            CHECK(data4[4] == 0);
        }

        SECTION("Outline and Fill") {
            {
                auto painter = bmp->BeginDraw();
                painter->SetPenSolid(1, 1);
                painter->SetBrushIndex(2);
                painter->DrawRect({{1, 1}, {4, 4}});
            }
            // Check fill (center)
            for (int y = 2; y <= 3; ++y) {
                auto data = bmp->GetLineData(y);
                for (int x = 2; x <= 3; ++x) {
                    CHECK(data[x] == 2);
                }
            }
            // Check outline
            auto data1 = bmp->GetLineData(1);
            auto data4 = bmp->GetLineData(4);
            for (int x = 1; x <= 4; ++x) {
                CHECK(data1[x] == 1);
                CHECK(data4[x] == 1);
            }
            for (int y = 1; y <= 4; ++y) {
                auto data = bmp->GetLineData(y);
                CHECK(data[1] == 1);
                CHECK(data[4] == 1);
            }
        }
    }

    SECTION("DrawEllipse") {
        SECTION("Circle Outline") {
            {
                auto painter = bmp->BeginDraw();
                painter->SetPenSolid(1, 1);
                painter->DrawEllipse({{0, 0}, {9, 9}});
            }
            // Center is (4, 4). Radius is 4.5? Rect size is 9, so radius is 4.
            // Points should be approximately on a circle.
            auto data0 = bmp->GetLineData(0);
            CHECK(data0[4] == 1); // Top
            auto data8 = bmp->GetLineData(8);
            CHECK(data8[4] == 1); // Bottom
            auto data4 = bmp->GetLineData(4);
            CHECK(data4[0] == 1); // Left
            CHECK(data4[8] == 1); // Right
            CHECK(data4[4] == 0); // Center should be empty
        }

        SECTION("Circle Fill") {
            {
                auto painter = bmp->BeginDraw();
                painter->SetNoPen();
                painter->SetBrushIndex(2);
                painter->DrawEllipse({{0, 0}, {9, 9}});
            }
            auto data4 = bmp->GetLineData(4);
            CHECK(data4[4] == 2); // Center should be filled
        }
    }
}

TEST_CASE("Painter - State Management", "[painter]") {
    auto bmp = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
    auto painter = bmp->BeginDraw();
    
    painter->SetPenSolid(1, 1);
    painter->SetBrushIndex(2);
    
    SECTION("SetNoPen") {
        painter->SetNoPen();
        painter->DrawPoint({0, 0});
        auto data = bmp->GetLineData(0);
        CHECK(data[0] == 0);
    }
    
    SECTION("SetNoBrush") {
        painter->SetNoBrush();
        painter->DrawRect({{0, 0}, {5, 5}});
        // Center of rect should be empty
        auto data = bmp->GetLineData(2);
        CHECK(data[2] == 0);
    }
}

TEST_CASE("Painter - Clipping", "[painter]") {
    auto bmp = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
    (void)bmp->SetPaletteColor(1, Color(255, 0, 0));
    
    auto painter = bmp->BeginDraw();
    painter->SetPenSolid(1, 1);
    
    SECTION("DrawPoint outside") {
        painter->DrawPoint({10, 10});
        painter->DrawPoint({static_cast<BitmapSizeType>(-1), static_cast<BitmapSizeType>(-1)}); // Large unsigned values
        // Nothing should crash
    }
    
    SECTION("DrawLine clipping") {
        painter->DrawLine({5, 5}, {15, 15});
        // Should draw from (5,5) to (9,9)
        for (int i = 5; i <= 9; ++i) {
            auto data = bmp->GetLineData(i);
            CHECK(data[i] == 1);
        }
    }

    SECTION("DrawRect clipping") {
        painter->SetNoPen();
        painter->SetBrushIndex(2);
        painter->DrawRect({{5, 5}, {10, 10}}); // Rect goes to (14, 14), but target is 10x10
        for (int y = 5; y <= 9; ++y) {
            auto data = bmp->GetLineData(y);
            for (int x = 5; x <= 9; ++x) {
                CHECK(data[x] == 2);
            }
        }
    }
}

TEST_CASE("Painter - DrawSurface", "[painter]") {
    auto src = Bitmap::Create({5, 5}, DrawableSurface::BitDepth::DEPTH_8, 2);
    (void)src->SetPaletteColor(1, Color(255, 255, 255));
    auto srcData = src->GetLineData(0);
    srcData[0] = 1;

    auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 2);
    (void)dst->SetPaletteColor(1, Color(255, 255, 255));
    
    auto painter = dst->BeginDraw();

    SECTION("Normal DrawSurface") {
        painter->DrawSurface(*src, {{0, 0}, {5, 5}}, {2, 2});
        auto data = dst->GetLineData(2);
        CHECK(data[2] == 1);
    }

    SECTION("Clipping DrawSurface") {
        painter->DrawSurface(*src, {{0, 0}, {5, 5}}, {8, 8});
        // Source is 5x5, starting at (8,8) in 10x10.
        // It should draw pixels at (8,8) up to (9,9).
        auto data8 = dst->GetLineData(8);
        CHECK(data8[8] == 1);
        
        // Check out of bounds doesn't crash
        painter->DrawSurface(*src, {{0, 0}, {5, 5}}, {10, 10});
        painter->DrawSurface(*src, {{0, 0}, {5, 5}}, {static_cast<BitmapSizeType>(-2), static_cast<BitmapSizeType>(-2)});
    }

    SECTION("Sub-rect DrawSurface") {
        auto src2 = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 2);
        (void)src2->SetPaletteColor(1, Color(255, 255, 255));
        auto srcData = src2->GetLineData(5);
        srcData[5] = 1; // Point at (5,5)

        painter->DrawSurface(*src2, {{5, 5}, {2, 2}}, {0, 0});
        auto data = dst->GetLineData(0);
        CHECK(data[0] == 1);
    }
}
