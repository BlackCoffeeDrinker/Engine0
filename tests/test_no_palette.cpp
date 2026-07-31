#include <catch2/catch_all.hpp>
#include <Engine/Resource/Bitmap.hpp>
#include <Engine/Platform/Painter.hpp>
#include <Engine/Platform/DrawableSurface.hpp>
#include <Engine/DefaultBitmapHelpers.hpp>

using namespace e00;

namespace {
std::vector<uint8_t> ReadIndexLine(const Bitmap &bmp, BitmapSizeType y, BitmapSizeType width) {
    std::vector<uint8_t> buffer(width);
    bmp.ReadLineInto(y, 0, width, DrawableSurface::TargetInformation{DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, nullptr, {}, {}}, buffer);
    return buffer;
}
}// namespace

TEST_CASE("Painter - DrawSurface NO_PALETTE", "[painter][no_palette]") {
    auto src = Bitmap::Create({5, 5}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
    {
        auto srcPainter = src->BeginDraw();
        srcPainter->SetPenSolid(1, 42); // Index 42
        srcPainter->DrawPoint({0, 0});
    }

    SECTION("NO_PALETTE to DEPTH_8") {
        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8, 256);
        auto painter = dst->BeginDraw();
        painter->BlitSurface(*src, {{0, 0}, {5, 5}}, {2, 2});
        auto data = ReadIndexLine(*dst, 2, 10);
        CHECK(data[2] == 42);
    }

    SECTION("NO_PALETTE to NO_PALETTE") {
        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
        auto painter = dst->BeginDraw();
        painter->BlitSurface(*src, {{0, 0}, {5, 5}}, {2, 2});
        auto data = ReadIndexLine(*dst, 2, 10);
        CHECK(data[2] == 42);
    }

    SECTION("DEPTH_8 to NO_PALETTE") {
        auto src8 = Bitmap::Create({5, 5}, DrawableSurface::BitDepth::DEPTH_8, 256);
        {
            auto src8Painter = src8->BeginDraw();
            src8Painter->SetPenSolid(1, 66);
            src8Painter->DrawPoint({0, 0});
        }

        auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
        auto painter = dst->BeginDraw();
        painter->BlitSurface(*src8, {{0, 0}, {5, 5}}, {2, 2});
        auto data = ReadIndexLine(*dst, 2, 10);
        CHECK(data[2] == 66);
    }

    SECTION("HasSamePalette") {
        auto src8 = Bitmap::Create({5, 5}, DrawableSurface::BitDepth::DEPTH_8, 256);
        auto dst = Bitmap::Create({5, 5}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
        CHECK(src8->HasSamePalette(*dst));
        CHECK(dst->HasSamePalette(*src8));
    }
}

TEST_CASE("Painter - PutPixel NO_PALETTE", "[painter][no_palette]") {
    auto dst = Bitmap::Create({10, 10}, DrawableSurface::BitDepth::DEPTH_8_NO_PALETTE, 0);
    auto painter = dst->BeginDraw();

    SECTION("PutPixel with Index") {
        painter->SetPenSolid(1, 123); // Index 123
        painter->DrawPoint({1, 1});
        auto data = ReadIndexLine(*dst, 1, 10);
        CHECK(data[1] == 123);
    }
}
