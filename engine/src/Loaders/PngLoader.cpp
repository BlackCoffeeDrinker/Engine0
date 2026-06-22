
#include "PngLoader.hpp"
#include "BitmapData.hpp"

#ifdef DJGPP
typedef int off_t;
#endif

#include <bit>
#include <zlib.h>

namespace {
constexpr uint32_t GetTypeInt32(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return a | (b << 8) | (c << 16) | (d << 24);
}

constexpr uint32_t GetTypeInt32(std::array<uint8_t, 4> c) {
  return c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24);
}

e00::DrawableSurface::BitDepth GetBitDepth(uint8_t bitDepth) {
  switch (bitDepth) {
    case 1: return e00::Bitmap::BitDepth::DEPTH_1;
    case 8: return e00::Bitmap::BitDepth::DEPTH_8;
    case 16: return e00::Bitmap::BitDepth::DEPTH_16;
    case 32: return e00::Bitmap::BitDepth::DEPTH_32;
    default: break;
  }

  return e00::DrawableSurface::BitDepth::DEPTH_INVALID;
}

struct PNGContext {
  z_stream strm;
  bool initialized = false;

  std::vector<uint8_t> decompressedData;

  PNGContext() : strm{} {
    strm.zalloc = nullptr;
    strm.zfree = nullptr;
    strm.opaque = nullptr;
    strm.avail_in = 0;
    strm.next_in = nullptr;
    if (const auto res = inflateInit(&strm); res == Z_OK) {
      initialized = true;
    } else {
      e00::GetDefaultLogger().Error(e00::source_location::current(), "inflateInit failed with error {}", res);
    }
  }

  ~PNGContext() {
    if (initialized) {
      inflateEnd(&strm);
    }
  }
};

struct PNGChunk {
  uint32_t size = 0;
  std::array<uint8_t, 4> type{0, 0, 0, 0};
  uint32_t crc = 0;

  size_t streamDataPositon = 0;

  [[nodiscard]] std::string_view StrType() const {
    return {reinterpret_cast<const char *>(type.data()), 4};
  }

  explicit operator bool() const {
    return crc != 0 && size > 0;
  }

  std::error_code skip(e00::Stream &stream) const {
    return stream.SeekTo(streamDataPositon + size + 4);
  }

  [[nodiscard]] uint32_t TypeAsInt32() const {
    return GetTypeInt32(type);
  }
};

// Generate the CRC lookup table
auto GenerateCRCTable() {
  static constexpr uint32_t POLYNOMIAL = 0xEDB88320;

  std::array<uint32_t, 256> crc_table{};

  for (uint32_t i = 0; i < 256; i++) {
    uint32_t crc = i;

    for (int j = 0; j < 8; j++) {
      if (crc & 1)
        crc = (crc >> 1) ^ POLYNOMIAL;
      else
        crc >>= 1;
    }

    crc_table[i] = crc;
  }

  return crc_table;
}

uint32_t ReadPNGUint32(e00::Stream &stream) {
  if constexpr (std::endian::native == std::endian::little) {
    std::array<uint8_t, 4> size{};
    stream.Read(size);

    return (static_cast<uint32_t>(size[0]) << 24) |
           (static_cast<uint32_t>(size[1]) << 16) |
           (static_cast<uint32_t>(size[2]) << 8) |
           (static_cast<uint32_t>(size[3]));
  } else if constexpr (std::endian::native == std::endian::big) {
    uint32_t size = 0;
    stream.Read(size);
    return size;
  }

  return 0;
}

PNGChunk ReadChunk(e00::Stream &stream) {
  static std::array<uint32_t, 256> pngCrcTable = GenerateCRCTable();

  PNGChunk chunk{};

  chunk.size = ReadPNGUint32(stream);
  if (const auto ec = stream.Read(chunk.type)) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to read PNG chunk type: {}", ec.message());
    return {};
  }

  chunk.streamDataPositon = stream.Position();

  // Compute CRC of the type and data
  uint32_t crc = 0xFFFFFFFF;// Initialize with all bits set

  // Add the type to the CRC check
  crc = (crc >> 8) ^ pngCrcTable[(crc ^ chunk.type[0]) & 0xFF];
  crc = (crc >> 8) ^ pngCrcTable[(crc ^ chunk.type[1]) & 0xFF];
  crc = (crc >> 8) ^ pngCrcTable[(crc ^ chunk.type[2]) & 0xFF];
  crc = (crc >> 8) ^ pngCrcTable[(crc ^ chunk.type[3]) & 0xFF];

  // Read data bits
  static constexpr size_t CRC_BUF_SIZE = 4096;
  std::array<uint8_t, CRC_BUF_SIZE> crcBuffer{};
  size_t remaining = chunk.size;
  while (remaining > 0) {
    const size_t toRead = std::min(remaining, CRC_BUF_SIZE);
    if (const auto ec = stream.Read(toRead, crcBuffer.data())) {
      e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to read PNG chunk data for CRC: {}", ec.message());
      return {};
    }

    for (size_t i = 0; i < toRead; ++i) {
      crc = (crc >> 8) ^ pngCrcTable[(crc ^ crcBuffer[i]) & 0xFF];
    }
    remaining -= toRead;
  }

  // Read the CRC
  chunk.crc = ReadPNGUint32(stream);

  // Make sure they match
  if ((crc ^ 0xFFFFFFFF) != chunk.crc) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "PNG chunk CRC mismatch");
    return {};
  }

  if (const auto ec = stream.SeekTo(chunk.streamDataPositon)) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to seek to start of chunk data: {}", ec.message());
    return {};
  }

  return chunk;
}

std::error_code ProcessPLTEData(e00::Stream &stream, e00::Bitmap &bitmap, const uint32_t size) {
  if (size % 3 != 0) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Palette of size of {} is not divisible by 3 !", size);
    return std::make_error_code(std::errc::invalid_argument);
  }

  if (size > 256 * 3) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Palette of size {} is too big!", size / 3);
    return std::make_error_code(std::errc::invalid_argument);
  }

  e00::FixedPalette colors(size / 3u);

  for (auto &color: colors) {
    if (const auto ec = stream.Read(color.red)) return ec;
    if (const auto ec = stream.Read(color.green)) return ec;
    if (const auto ec = stream.Read(color.blue)) return ec;
  }

  bitmap.SetPalette(colors);
  return {};
}

std::error_code ProcessIDATData(e00::Stream &stream, PNGContext &context, const uint32_t size) {
  if (!context.initialized) {
    return std::make_error_code(std::errc::not_enough_memory);
  }

  std::vector<uint8_t> compressedData;
  try {
    compressedData.resize(size);
  } catch (const std::bad_alloc &) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to allocate {} bytes for IDAT chunk", size);
    return std::make_error_code(std::errc::not_enough_memory);
  }

  if (const auto ec = stream.Read(compressedData)) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to read IDAT chunk: {} (requested {} bytes, available {} bytes)", ec.message(), size, stream.AvailableToRead());
    return ec;
  }

  context.strm.avail_in = static_cast<uInt>(compressedData.size());
  context.strm.next_in = compressedData.data();

  std::array<uint8_t, 4096> outputBuffer{};

  do {
    context.strm.avail_out = static_cast<uInt>(outputBuffer.size());
    context.strm.next_out = outputBuffer.data();

    if (const auto result = inflate(&context.strm, Z_NO_FLUSH);
        result < 0 && result != Z_BUF_ERROR) {
      e00::GetDefaultLogger().Error(e00::source_location::current(), "Inflate error: {} (code {})", context.strm.msg ? context.strm.msg : "unknown", result);

      context.strm.next_out = nullptr;
      context.strm.next_in = nullptr;

      return std::make_error_code(std::errc::invalid_argument);
    }

    const auto produced = outputBuffer.size() - context.strm.avail_out;
    context.decompressedData.insert(
        context.decompressedData.end(),
        outputBuffer.begin(),
        outputBuffer.begin() + static_cast<std::ptrdiff_t>(produced));

  } while (context.strm.avail_out == 0);

  if (context.strm.avail_in != 0) {
    e00::GetDefaultLogger().Error(e00::source_location::current(), "Failed to inflate all IDAT data ({} bytes remaining)", context.strm.avail_in);
    return std::make_error_code(std::errc::invalid_argument);
  }

  context.strm.next_out = nullptr;
  context.strm.next_in = nullptr;

  return {};
}
// ... existing code ...
uint8_t PaethPredictor(uint8_t left, uint8_t above, uint8_t upperLeft) {
  const int p = static_cast<int>(left) + static_cast<int>(above) - static_cast<int>(upperLeft);
  const int pa = std::abs(p - static_cast<int>(left));
  const int pb = std::abs(p - static_cast<int>(above));
  const int pc = std::abs(p - static_cast<int>(upperLeft));

  if (pa <= pb && pa <= pc) {
    return left;
  }

  if (pb <= pc) {
    return above;
  }

  return upperLeft;
}

std::error_code ApplyPNGFiltersToBitmap(const PNGContext &context, e00::Bitmap &bitmap) {
  const auto size = bitmap.Size();
  const auto rowBytes = static_cast<size_t>(size.x);
  const auto expectedSize = static_cast<size_t>(size.y) * (rowBytes + 1);

  if (context.decompressedData.size() < expectedSize) {
    e00::GetDefaultLogger().Error(
        e00::source_location::current(),
        "PNG decompressed data too small: got {}, expected at least {}",
        context.decompressedData.size(),
        expectedSize);

    return std::make_error_code(std::errc::invalid_argument);
  }

  std::vector<uint8_t> previousRow(rowBytes, 0);
  std::vector<uint8_t> currentRow(rowBytes, 0);

  size_t srcOffset = 0;

  for (e00::BitmapSizeType y = 0; y < size.y; ++y) {
    constexpr size_t bytesPerPixel = 1;
    const auto filterType = context.decompressedData[srcOffset++];

    std::copy_n(
        context.decompressedData.data() + srcOffset,
        rowBytes,
        currentRow.data());

    srcOffset += rowBytes;

    switch (filterType) {
      case 0:
        // None
        break;

      case 1:
        // Sub
        for (size_t x = 0; x < rowBytes; ++x) {
          const auto left = x >= bytesPerPixel ? currentRow[x - bytesPerPixel] : uint8_t{0};
          currentRow[x] = static_cast<uint8_t>(currentRow[x] + left);
        }
        break;

      case 2:
        // Up
        for (size_t x = 0; x < rowBytes; ++x) {
          currentRow[x] = static_cast<uint8_t>(currentRow[x] + previousRow[x]);
        }
        break;

      case 3:
        // Average
        for (size_t x = 0; x < rowBytes; ++x) {
          const auto left = x >= bytesPerPixel ? currentRow[x - bytesPerPixel] : uint8_t{0};
          const auto above = previousRow[x];
          currentRow[x] = static_cast<uint8_t>(currentRow[x] + ((static_cast<uint16_t>(left) + above) / 2));
        }
        break;

      case 4:
        // Paeth
        for (size_t x = 0; x < rowBytes; ++x) {
          const auto left = x >= bytesPerPixel ? currentRow[x - bytesPerPixel] : uint8_t{0};
          const auto above = previousRow[x];
          const auto upperLeft = x >= bytesPerPixel ? previousRow[x - bytesPerPixel] : uint8_t{0};
          currentRow[x] = static_cast<uint8_t>(currentRow[x] + PaethPredictor(left, above, upperLeft));
        }
        break;

      default:
        e00::GetDefaultLogger().Error(
            e00::source_location::current(),
            "Unsupported PNG filter type {}",
            static_cast<int>(filterType));

        return std::make_error_code(std::errc::invalid_argument);
    }

    bitmap.WriteLine_N(y, currentRow, rowBytes);

    previousRow.swap(currentRow);
    std::ranges::fill(currentRow, uint8_t{0});
  }

  return {};
}


}// namespace

namespace e00::impl {

bool PNGLoader::SupportsOption(type_t optionTypeid) const {
  return type_id<DiscardPalette>() == optionTypeid;
}
/**
 * Determines if the provided data represents a valid PNG file.
 *
 * This method analyzes the given stream to check for the presence
 * of the PNG file signature and other required structures. It does
 * not fully decode the file but performs a lightweight validation
 * to figure out if the data is likely to be a PNG.
 *
 * @param stream The stream containing the data to be validated.
 * @return True if the data appears to be a valid PNG, false otherwise.
 */
bool PNGLoader::CanLoad(const LoadContext &context) {
  // Define the PNG signature
  constexpr std::array<uint8_t, 8> PNG_SIGNATURE = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

  // Ensure the stream has enough data to check the signature
  if (context.stream.Size() < PNG_SIGNATURE.size()) {
    return false;
  }

  // Read the first 8 bytes of the stream
  // Create a buffer to store the read bytes
  std::array<uint8_t, PNG_SIGNATURE.size()> buffer{};
  if (context.stream.Read(buffer)) {
    return false;
  }

  // Compare the read bytes with the PNG signature
  if (buffer != PNG_SIGNATURE) {
    return false;
  }

  // IHDR needs to be the first chunk
  const auto firstChuck = ReadChunk(context.stream);
  return firstChuck.StrType() == "IHDR";
}

/**
 * Reads and loads PNG data from the provided stream.
 *
 * The method parses the PNG file structure, verifies its validity,
 * extracts image dimensions, and processes chunks such as IHDR, PLTE,
 * and IDAT to construct a bitmap representation of the image.
 *
 * @param context the load context
 * @return A result containing either a unique pointer to a bitmap if
 *         the loading is successful, or an error code if an error occurs.
 */
ResourceLoader::Result PNGLoader::ReadLoad(const LoadContext &context) {
  if (const auto ec = context.stream.SeekTo(0)) {
    return ec;
  }

  if (!CanLoad(context)) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  if (const auto ec = context.stream.SeekTo(8)) {
    return ec;
  }

  // The first chunk needs to be IHDR
  const auto IHDR = ReadChunk(context.stream);
  if (IHDR.StrType() != "IHDR") {// TODO: Remove this check since CanLoad does this ?
    return std::make_error_code(std::errc::invalid_argument);
  }

  const auto width = ReadPNGUint32(context.stream);
  const auto height = ReadPNGUint32(context.stream);

  // Make sure the size is okay
  if (width > std::numeric_limits<BitmapSizeType>::max() || height > std::numeric_limits<BitmapSizeType>::max()) {
    GetDefaultLogger().Error(source_location::current(), "Bitmap size ({} x {}) is too large", width, height);
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Read IHDR
  uint8_t bitDepth, colorType, compressionType, filterMethod, interlaceMethod;
  if (const auto ec = context.stream.Read(bitDepth)) return ec;
  if (const auto ec = context.stream.Read(colorType)) return ec;
  if (const auto ec = context.stream.Read(compressionType)) return ec;
  if (const auto ec = context.stream.Read(filterMethod)) return ec;
  if (const auto ec = context.stream.Read(interlaceMethod)) return ec;

  if (const auto ec = IHDR.skip(context.stream)) return ec;

  // TODO: Support other formats
  if (colorType != 3 || bitDepth != 8) {
    GetDefaultLogger().Error(source_location::current(), "Unsupported PNG format (for now)");
    return std::make_error_code(std::errc::invalid_argument);
  }

  // png state
  PNGContext png_context;

  // make the bitmap that will hold the data
  auto bitmapData = Bitmap::Create(
      Vec2D<BitmapSizeType>(width, height),
      GetBitDepth(bitDepth));

  // Read the chunks
  while (!context.stream.AtEnd()) {
    if (const auto chunk = ReadChunk(context.stream)) {
      switch (chunk.TypeAsInt32()) {
        case GetTypeInt32('P', 'L', 'T', 'E'):
          if (const auto ec = ProcessPLTEData(context.stream, *bitmapData, chunk.size)) {
            GetDefaultLogger().Error(source_location::current(), "Failed to process PLTE chunk: {}", ec.message());
            return ec;
          }
          break;

        case GetTypeInt32('I', 'D', 'A', 'T'):
          if (const auto ec = ProcessIDATData(context.stream, png_context, chunk.size)) {
            GetDefaultLogger().Error(source_location::current(), "Failed to process IDAT chunk: {}", ec.message());
            return ec;
          }
          break;

        default:
          GetDefaultLogger().Info(source_location::current(), "Skipping PNG chunk {}", chunk.StrType());
      }

      if (const auto ec = chunk.skip(context.stream)) {
        GetDefaultLogger().Error(source_location::current(), "Failed to skip PNG chunk");
        return ec;
      }
    } else {
      // This might be valid for IEND
      if (chunk.TypeAsInt32() == GetTypeInt32('I', 'E', 'N', 'D')) {
        break;
      }
      GetDefaultLogger().Error(source_location::current(), "Failed to read PNG chunk");
      return std::make_error_code(std::errc::invalid_argument);
    }
  }

  if (const auto ec = ApplyPNGFiltersToBitmap(png_context, *bitmapData)) {
    GetDefaultLogger().Error(source_location::current(), "Failed to apply PNG filters: {}", ec.message());
    return ec;
  }

  // Need to discard the palette if it was loaded
  if (context.ContainsOption<DiscardPalette>()) {
    bitmapData->DiscardPalette();
  }

  return std::move(bitmapData);
}
}// namespace e00::impl
