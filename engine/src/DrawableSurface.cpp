#include "PrivateInclude.hpp"

namespace e00 {
DrawableSurface::~DrawableSurface() = default;

std::error_code DrawableSurface::SaveToBMP(WritableStream &writableStream) const {
  constexpr std::array magic = {'B', 'M'};
  const auto bitDepth = GetBitDepth();
  const auto numBits = DepthEnumToBits(bitDepth);
  const auto width = Size().x;
  const auto height = Size().y;

  if (width == 0 || height == 0) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Calculate row tracking constraints (BMP rows must align to 4-byte boundaries)
  const size_t bytesPerPixel = (numBits + 7) / 8;
  const size_t rawRowSize = (bitDepth == BitDepth::DEPTH_1) ? ((width + 7) / 8) : (width * bytesPerPixel);
  const size_t pad = ((rawRowSize % 4) ? (4 - (rawRowSize % 4)) : 0);
  const size_t paddedRowSize = rawRowSize + pad;
  const size_t imageSizePadded = paddedRowSize * height;

  // High/True color formats use BITMAPV3HEADER (56 bytes) to append bitfield color masks
  const bool useBitfields = (bitDepth == BitDepth::DEPTH_16 || bitDepth == BitDepth::DEPTH_32);
  const uint32_t infoHeaderSize = useBitfields ? 56 : 40;

  const size_t numPaletteColors = (bitDepth == BitDepth::DEPTH_8) ? GetNumberOfColorsInPalette() : (bitDepth == BitDepth::DEPTH_8_NO_PALETTE ? 256 : 0);
  const size_t offsetAfterHeader = 14 + infoHeaderSize + (numPaletteColors * 4);

  /* 1. Write the standard 14-byte File Header */
  writableStream.Write(magic);                                                    // 2
  writableStream.WriteLittleEndian<uint32_t>(offsetAfterHeader + imageSizePadded);// 4 File Size
  writableStream.WriteLittleEndian<uint16_t>(0);                                  // 2 Reserved
  writableStream.WriteLittleEndian<uint16_t>(0);                                  // 2 Reserved
  writableStream.WriteLittleEndian<uint32_t>(offsetAfterHeader);                  // 4 Offset to pixels

  /* 2. Write the Info Header */
  writableStream.WriteLittleEndian<uint32_t>(infoHeaderSize);
  writableStream.WriteLittleEndian<int32_t>(width);
  writableStream.WriteLittleEndian<int32_t>(height);
  writableStream.WriteLittleEndian<uint16_t>(1);
  writableStream.WriteLittleEndian<uint16_t>(numBits);

  // Compression Mode: 0 = BI_RGB, 3 = BI_BITFIELDS (Required for 16/32bpp)
  const uint32_t compressionType = useBitfields ? 3 : 0;
  writableStream.WriteLittleEndian<uint32_t>(compressionType);

  writableStream.WriteLittleEndian<uint32_t>(0);
  writableStream.WriteLittleEndian<int32_t>(0);
  writableStream.WriteLittleEndian<int32_t>(0);
  writableStream.WriteLittleEndian<uint32_t>(numPaletteColors);
  writableStream.WriteLittleEndian<uint32_t>(0);

  /* 3. Append Color Bitfields if using High/True Color */
  // We can populate standard, universally compatible Windows BMP masks here
  // 16-bit defaults to standard 5:6:5 RGB layout. 32-bit defaults to 8:8:8:0 RGB layout.
  if (useBitfields) {
    uint32_t rMask = 0, gMask = 0, bMask = 0;
    if (bitDepth == BitDepth::DEPTH_16) {
      rMask = 0xF800;
      gMask = 0x07E0;
      bMask = 0x001F;// standard 5:6:5 masks
    } else {
      rMask = 0x00FF0000;
      gMask = 0x0000FF00;
      bMask = 0x000000FF;// standard XRGB masks
    }
    writableStream.WriteLittleEndian<uint32_t>(rMask);
    writableStream.WriteLittleEndian<uint32_t>(gMask);
    writableStream.WriteLittleEndian<uint32_t>(bMask);
    writableStream.WriteLittleEndian<uint32_t>(0);// Alpha/Reserved Mask
  }

  /* 4. Write palette data if 8-bit (Blue, Green, Red, Reserved order) */
  if (bitDepth == BitDepth::DEPTH_8) {
    for (size_t i = 0; i < numPaletteColors; ++i) {
      const auto &color = GetColorFromPalette(i);
      writableStream.WriteLittleEndian<uint8_t>(color.blue);
      writableStream.WriteLittleEndian<uint8_t>(color.green);
      writableStream.WriteLittleEndian<uint8_t>(color.red);
      writableStream.WriteLittleEndian<uint8_t>(0);
    }
  } else if (bitDepth == BitDepth::DEPTH_8_NO_PALETTE) {
    for (size_t i = 0; i < 256; ++i) {
      writableStream.WriteLittleEndian<uint8_t>(static_cast<uint8_t>(i));
      writableStream.WriteLittleEndian<uint8_t>(static_cast<uint8_t>(i));
      writableStream.WriteLittleEndian<uint8_t>(static_cast<uint8_t>(i));
      writableStream.WriteLittleEndian<uint8_t>(0);
    }
  }

  /* 5. Extract and write the bitmap image pixels upside down */

  // Setup the TargetInformation layout contract matching our file depth parameters exactly
  TargetInformation targetInfo{};
  targetInfo.bit_depth = bitDepth;

  FixedPalette tempPalette;
  if (bitDepth == BitDepth::DEPTH_8) {
    tempPalette.resize(GetNumberOfColorsInPalette());
    for (size_t i = 0; i < tempPalette.size(); ++i) {
      tempPalette[i] = GetColorFromPalette(i);
    }
    targetInfo.palette = &tempPalette;
  }

  if (bitDepth == BitDepth::DEPTH_16) {
    targetInfo.shift = {11, 5, 0};
    targetInfo.mask = {0x1F, 0x3F, 0x1F};// 5:6:5 High Color contract
  } else if (bitDepth == BitDepth::DEPTH_32) {
    targetInfo.shift = {16, 8, 0};
    targetInfo.mask = {0xFF, 0xFF, 0xFF};// XRGB True Color contract
  }

  // Allocate a single-line scratchpad buffer in system RAM to process the extraction stream
  std::vector<uint8_t> lineBuffer(rawRowSize);

  for (auto y = height; y != 0; --y) {
    ReadLineInto(y - 1, 0, width, targetInfo, lineBuffer);

    // Flush the transcoded line straight to disk
    writableStream.Write(lineBuffer.size(), lineBuffer.data());

    // Append 4-byte row structure alignment padding
    for (size_t i = 0; i < pad; ++i) {
      writableStream.WriteLittleEndian<uint8_t>(0);
    }
  }

  return {};
}


}// namespace e00
