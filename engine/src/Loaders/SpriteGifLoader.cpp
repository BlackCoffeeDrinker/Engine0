
#include "SpriteGifLoader.hpp"

namespace {
constexpr std::array<std::uint8_t, 6> GIF87a = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61};
constexpr std::array<std::uint8_t, 6> GIF89a = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};
constexpr auto LOGICAL_SCREEN_DESCRIPTOR_SIZE = 7;

// Helper class to read bits from the compressed data
class BitStreamReader {
  const std::vector<uint8_t> &data_;
  size_t byte_pos_;
  int bit_pos_;

public:
  explicit BitStreamReader(const std::vector<uint8_t> &data)
      : data_(data), byte_pos_(0), bit_pos_(0) {}

  // Read the next 'num_bits' bits as an integer
  uint16_t ReadBits(const int num_bits) {
    uint16_t value = 0;
    for (int i = 0; i < num_bits; ++i) {
      if (byte_pos_ >= data_.size()) {
        throw std::runtime_error("Unexpected end of data");
      }
      const uint8_t current_byte = data_[byte_pos_];
      const uint8_t bit = (current_byte >> bit_pos_) & 1;
      value |= (bit << i);
      ++bit_pos_;
      if (bit_pos_ >= 8) {
        bit_pos_ = 0;
        ++byte_pos_;
      }
    }
    return value;
  }

  [[nodiscard]] bool EndOfStream() const {
    return byte_pos_ >= data_.size();
  }
};

struct ApplicationExtension {
  std::string identifier;
  std::string authCode;
  std::vector<uint8_t> data;
  uint16_t seenAtFrame{0};
};

struct PackedField {
  bool globalColorTableFlag;     // Bit 7
  uint8_t colorResolution;       // Bits 6-4
  bool sortFlag;                 // Bit 3
  uint8_t sizeOfGlobalColorTable;// Bits 2-0
};

struct LogicalScreenDescriptor {
  uint32_t width;
  uint32_t height;
  PackedField fields;
  uint8_t background_color_index;
  uint8_t pixel_aspect_ratio;
};

struct GifImageContext {
  e00::RectT<uint16_t> dirtyRect;
  e00::FixedPalette palette;
  bool interlaceFlag;
  bool sortFlag;
};

struct GifContext {
  enum class GifDisposalMethod : uint8_t {
    UNSPECIFIED = 0,
    DO_NOT_DISPOSE = 1,
    RESTORE_TO_BACKGROUND = 2,
    RESTORE_TO_PREVIOUS = 3,
    RESERVED
  };

  uint8_t version;
  LogicalScreenDescriptor lsd;
  e00::FixedPalette globalPalette;
  std::string commentData;
  std::vector<ApplicationExtension> appData;
  std::string textData;

  GifDisposalMethod disposalMethod;
  bool userInputFlag;
  bool transparentColorFlag;
  std::chrono::milliseconds delayTime;
  uint8_t transparentColorIndex;

  uint16_t numFrames{0};
};

// Helper function: Decode LZW compressed data
std::vector<uint8_t> LZWDecompress(const std::vector<uint8_t> &compressedData, const uint8_t minCodeSize) {
  std::vector<uint8_t> output;
  if (minCodeSize < 2 || minCodeSize > 8) {
    throw std::invalid_argument("Invalid minimum code size");
  }

  BitStreamReader bitReader(compressedData);

  // Initialize the dictionary
  const int clearCode = 1 << minCodeSize;
  const int eoiCode = clearCode + 1;
  int nextCode = eoiCode + 1;
  int codeSize = minCodeSize + 1;
  constexpr int maxCodeSize = 12;
  constexpr int maxDictSize = 1 << maxCodeSize;

  std::vector<std::vector<uint8_t>> dictionary(maxDictSize);
  for (int i = 0; i < clearCode; ++i) {
    dictionary[i].push_back(static_cast<uint8_t>(i));
  }
  dictionary[clearCode].push_back(0);// Clear code
  dictionary[eoiCode].push_back(0);  // EOI code

  int previousCode = -1;
  while (!bitReader.EndOfStream()) {
    if (nextCode >= maxDictSize) {
      // Maximum dictionary size reached
      // In GIF, dictionary size does not wrap around,
      // So we stop adding new codes
    }

    uint16_t currentCode;
    try {
      currentCode = bitReader.ReadBits(codeSize);
    } catch (...) {
      break;// Reached the end of data
    }

    if (currentCode == clearCode) {
      // Reset the dictionary
      dictionary.clear();
      dictionary.resize(maxDictSize);
      for (int i = 0; i < clearCode; ++i) {
        dictionary[i].push_back(static_cast<uint8_t>(i));
      }
      dictionary[clearCode].push_back(0);// Clear code
      dictionary[eoiCode].push_back(0);  // EOI code
      nextCode = eoiCode + 1;
      codeSize = minCodeSize + 1;
      previousCode = -1;
      continue;
    }

    if (currentCode == eoiCode) {
      break;// End of Information
    }

    if (currentCode < nextCode && !dictionary[currentCode].empty()) {
      // Existing code
      const auto &entry = dictionary[currentCode];
      output.insert(output.end(), entry.begin(), entry.end());

      if (previousCode != -1 && nextCode < maxDictSize) {
        // Add new entry to the dictionary
        std::vector<uint8_t> newEntry = dictionary[previousCode];
        newEntry.push_back(entry[0]);
        dictionary[nextCode++] = newEntry;

        // Increase code size if necessary
        if (nextCode == (1 << codeSize) && codeSize < maxCodeSize) {
          ++codeSize;
        }
      }
      previousCode = currentCode;
    } else if (currentCode == nextCode && previousCode != -1) {
      // Special case: currentCode is nextCode
      std::vector<uint8_t> newEntry = dictionary[previousCode];
      newEntry.push_back(dictionary[previousCode][0]);
      output.insert(output.end(), newEntry.begin(), newEntry.end());

      if (nextCode < maxDictSize) {
        dictionary[nextCode++] = newEntry;

        // Increase code size if necessary
        if (nextCode == (1 << codeSize) && codeSize < maxCodeSize) {
          ++codeSize;
        }
      }
      previousCode = currentCode;
    } else {
      throw std::runtime_error("Invalid LZW code encountered");
    }
  }

  return output;
}

// Helper function: Apply interlacing
std::vector<uint8_t> ApplyInterlace(const std::vector<uint8_t> &data, const uint32_t width, const uint32_t height) {
  std::vector<uint8_t> result(data.size());

  // Interlacing passes
  constexpr std::array<std::pair<uint8_t, uint8_t>, 4> passes = {{
      {0, 8},// Starting at row 0, step size 8
      {4, 8},// Starting at row 4, step size 8
      {2, 4},// Starting at row 2, step size 4
      {1, 2},// Starting at row 1, step size 2
  }};

  ptrdiff_t sourceIndex = 0;
  for (const auto &[start, step]: passes) {
    for (uint32_t row = start; row < height; row += step) {
      std::copy_n(data.begin() + sourceIndex, width, result.begin() + row * width);
      sourceIndex += width;
    }
  }

  return result;
}

/**
 * Reads a GIF palette with `numColors` colors
 * 
 * @param stream the stream to read from
 * @param numColors the number of RGB triplets to read
 * @param output the output palette
 * @return any errors
 */
std::error_code ReadPalette(e00::Stream &stream, const uint32_t numColors, e00::FixedPalette &output) {
  if (numColors > 0 && numColors <= 256) {
    output.resize(static_cast<int>(numColors));

    // In a more modern engine, we might read all 3 * numOfColors in one go, but do not here,
    // for memory pressure constraints
    for (auto i = 0; i < numColors; i++) {
      std::array<uint8_t, 3> raw_palette{};
      if (const auto ec = stream.Read(raw_palette)) {
        return ec;
      }

      output[i] = e00::Color(raw_palette[0], raw_palette[1], raw_palette[2]);
    }
  }

  return {};
}

/**
 * Read the logical screen descriptor from the GIF stream
 * 
 * @param file the GIF stream to read the Logical Screen Descriptor
 * @return the empty Logical Screen Descriptor if there is an error
 */
LogicalScreenDescriptor ReadLogicalScreenDescriptor(e00::Stream &file) {
  std::array<std::uint8_t, LOGICAL_SCREEN_DESCRIPTOR_SIZE> lsd{};
  if (file.Read(lsd)) {
    return {};
  }

  PackedField pf{};
  pf.globalColorTableFlag = (lsd[4] >> 7) & 0x01;
  pf.colorResolution = (lsd[4] >> 4) & 0x07;
  pf.sortFlag = (lsd[4] >> 3) & 0x01;
  pf.sizeOfGlobalColorTable = lsd[4] & 0x07;

  // Extract and print Logical Screen Descriptor fields
  return {
      static_cast<uint32_t>(lsd[0] | lsd[1] << 8),
      static_cast<uint32_t>(lsd[2] | lsd[3] << 8),
      pf,
      (lsd[5]),
      (lsd[6]),
  };
}

/**
 * 
 * @param stream 
 * @param context 
 * @return 
 */
std::error_code ReadGraphicControlExtension(e00::Stream &stream, GifContext &context) {
  uint8_t block_size = 0;
  if (const auto ec = stream.Read(block_size)) {
    return ec;
  }

  if (block_size != 4) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  // Include terminator block
  std::array<uint8_t, 5> gceBlock{};
  if (const auto ec = stream.Read(gceBlock)) {
    return ec;
  }

  switch ((gceBlock[0] >> 2) & 0x07) {
    case 0:
      context.disposalMethod = GifContext::GifDisposalMethod::UNSPECIFIED;
      break;
    case 1:
      context.disposalMethod = GifContext::GifDisposalMethod::DO_NOT_DISPOSE;
      break;
    case 2:
      context.disposalMethod = GifContext::GifDisposalMethod::RESTORE_TO_BACKGROUND;
      break;
    case 3:
      context.disposalMethod = GifContext::GifDisposalMethod::RESTORE_TO_PREVIOUS;
      break;
    default:
      context.disposalMethod = GifContext::GifDisposalMethod::RESERVED;
      break;
  }

  context.userInputFlag = (gceBlock[0] >> 1) & 0x01;
  context.transparentColorFlag = gceBlock[0] & 0x01;
  context.delayTime = std::chrono::milliseconds(10 * (gceBlock[1] | (gceBlock[2] << 8)));
  context.transparentColorIndex = gceBlock[3];

  if (gceBlock[4] != 0) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  return {};
}

/**
 * 
 * @param stream 
 * @param context 
 * @return 
 */
std::error_code ReadCommentExtension(e00::Stream &stream, GifContext &context) {
  while (true) {
    uint8_t block_size = 0;
    if (const auto ec = stream.Read(block_size)) {
      return ec;
    }

    if (block_size == 0) {
      break;
    }

    // Allocate space
    context.commentData.resize(context.commentData.size() + block_size);

    // Read
    if (const auto ec = stream.Read(block_size, context.commentData.data() + context.commentData.size() - block_size)) {
      return ec;
    }
  }

  return {};
}

std::error_code ReadApplicationExtension(e00::Stream &stream, GifContext &context) {
  uint8_t extBlockSize = 0;
  if (const auto ec = stream.Read(extBlockSize)) {
    return ec;
  }

  std::vector<char> appData(extBlockSize);
  if (const auto ec = stream.Read(extBlockSize, appData.data())) {
    return ec;
  }

  if (extBlockSize < 11) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  ApplicationExtension ext;
  ext.seenAtFrame = context.numFrames;

  // The first 8 bytes in the appData are the Application Identifier (ASCII)
  ext.identifier = std::string(appData.data(), 8);

  // The next 3 bytes are the Application Authentication Code
  ext.authCode = std::string(appData.data() + 8, 3);

  // Process the remaining data blocks of the Application Extension
  while (true) {
    uint8_t block_size = 0;
    if (const auto ec = stream.Read(block_size)) {
      return ec;
    }

    if (block_size == 0) {
      // Block terminator encountered
      break;
    }

    std::vector<uint8_t> subBlockData(block_size);
    if (const auto ec = stream.Read(block_size, subBlockData.data())) {
      return ec;
    }

    // Process or store subBlockData as needed
    ext.data.insert(ext.data.end(), subBlockData.begin(), subBlockData.end());
  }

  context.appData.push_back(ext);

  return {};
}

/**
 * Skips over an unknown extension
 * 
 * @param stream the GIF stream
 * @return any errors
 */
std::error_code SkipExtension(e00::Stream &stream) {
  uint8_t blockSize = 0;
  if (const auto ec = stream.Read(blockSize)) {
    return ec;
  }

  /* Empty extension? Is not allowed, but just skip */
  if (blockSize == 0) {
    return {};
  }

  if (const auto ec = stream.SeekTo(stream.Position() + blockSize + 1)) {
    return ec;
  }

  return {};
}

/**
 * Read an extension block, the stream will be positioned after the extension
 * 
 * @param stream the stream the GIF extension block
 * @param context the image context
 * @return any error
 */
std::error_code ReadGifExtensionBlock(e00::Stream &stream, GifContext &context) {
  // Read the label of the extension
  uint8_t extensionLabel = 0;
  if (const auto ec = stream.Read(extensionLabel)) {
    return ec;
  }

  /* Read the extension */
  switch (extensionLabel) {
    case 0xF9:
      return ReadGraphicControlExtension(stream, context);
    case 0xFE:
      return ReadCommentExtension(stream, context);
    case 0xFF:
      return ReadApplicationExtension(stream, context);
    default:
      break;
  }

  return SkipExtension(stream);
}

/**
 * Reads a GIF image descriptor and saves the relevant information in the context
  * 
 * @param stream the stream to read the image descriptor from
 * @param image the image context to put the data in
 * @return any errors
 */
std::error_code ReadImageDescriptor(e00::Stream &stream, GifImageContext &image) {
  // Image Descriptor is 8 bytes + 1 for termination
  std::array<uint8_t, 9> imgDescriptor{};
  if (const auto ec = stream.Read(imgDescriptor)) {
    return ec;
  }

  // Parse the fields of the Image Descriptor
  uint16_t imageLeft = imgDescriptor[0] | (imgDescriptor[1] << 8);
  uint16_t imageTop = imgDescriptor[2] | (imgDescriptor[3] << 8);
  uint16_t imageWidth = imgDescriptor[4] | (imgDescriptor[5] << 8);
  uint16_t imageHeight = imgDescriptor[6] | (imgDescriptor[7] << 8);
  image.dirtyRect = {imageLeft, imageTop, imageWidth, imageHeight};

  // Decode packed fields
  const bool localColorTableFlag = (imgDescriptor[8] >> 7) & 0x01;
  image.interlaceFlag = (imgDescriptor[8] >> 6) & 0x01;
  image.sortFlag = (imgDescriptor[8] >> 5) & 0x01;
  const uint8_t localColorTableSize = imgDescriptor[8] & 0x07;

  // Optionally read the local color table if it exists
  if (localColorTableFlag) {
    const auto numColors = 2 << localColorTableSize;
    if (const auto ec = ReadPalette(stream, numColors, image.palette)) {
      return ec;
    }
  }

  return {};
}

/**
 * Read the LZW data from the GIF stream
 * 
 * @param stream the stream to read from
 * @return the read data, empty if there is an error
 */
std::vector<uint8_t> ReadCompressedImageData(e00::Stream &stream) {
  std::vector<uint8_t> compressedData;
  while (true) {
    uint8_t blockSize = 0;
    // Read the block size
    if (stream.Read(blockSize)) {
      return {};
    }

    // End of LZW data (blockSize == 0 signifies the end of image data)
    if (blockSize == 0) {
      break;
    }

    std::vector<uint8_t> blockData(blockSize);
    if (stream.Read(blockData)) {
      return {};
    }

    // Append the block data to the compressed data
    compressedData.insert(compressedData.end(), blockData.begin(), blockData.end());
  }

  return compressedData;
}

/**
 * 
 * @param stream the GIF file
 * @param context the gif context
 * @param finalSprite the sprite to put the images in
 * @return any errors
 */
std::error_code ReadImage(e00::Stream &stream, GifContext &context, const std::unique_ptr<e00::Sprite> &finalSprite) {
  GifImageContext imageContext{};

  // Step 1: Read and decode the image descriptor
  if (const auto ec = ReadImageDescriptor(stream, imageContext)) {
    return ec;
  }

  // Step 2: Read the minimum LZW code size
  uint8_t lzwMinCodeSize = 0;
  if (const auto ec = stream.Read(lzwMinCodeSize)) {
    return ec;
  }

  std::vector<uint8_t> decompressedData;

  /* Make sure compressed data is scoped very tight to free the data */
  {
    // Step 3: Read LZW image data blocks
    const auto compressedData = ReadCompressedImageData(stream);

    // Step 4: Decompress the LZW data
    // Handle LZW decompression (utilize a dedicated LZW decompression function)
    decompressedData = LZWDecompress(compressedData, lzwMinCodeSize);
  }

  // Step 5: Apply interlacing if necessary
  if (imageContext.interlaceFlag) {
    decompressedData = ApplyInterlace(decompressedData, imageContext.dirtyRect.size.x, imageContext.dirtyRect.size.y);
  }

  // Step 6: make the final frame
  if (imageContext.dirtyRect.size.Area() != decompressedData.size()) {
    // Decompression failed?
    return std::make_error_code(std::errc::invalid_argument);
  }
  
  auto image = e00::Bitmap::Create(
      imageContext.dirtyRect.size,
      e00::DrawableSurface::BitDepth::DEPTH_8,
      imageContext.palette);

  for (auto y = 0; y < imageContext.dirtyRect.size.y; ++y) {
    auto line_data = image->GetLineData(y);
    const auto lineStartIndex = y * imageContext.dirtyRect.size.x;

    memcpy(
        line_data.data(),
        decompressedData.data() + lineStartIndex,
        imageContext.dirtyRect.size.x);
  }

  if (imageContext.palette.empty()) {
    image->SetPalette(context.globalPalette);
  }

  context.numFrames++;

  return finalSprite->AddFrame(std::move(image), context.delayTime);
}

}// namespace

namespace e00::impl {
GifSpriteLoader::~GifSpriteLoader() = default;

bool GifSpriteLoader::SupportsOption(type_t optionTypeid) const {
  return type_id<DiscardPalette>() == optionTypeid;
}

bool GifSpriteLoader::CanLoad(const LoadContext& context) {
  std::array<std::uint8_t, 6> header{};
  if (context.stream.Read(header)) {
    return false;
  }

  return header == GIF87a || header == GIF89a;
}

ResourceLoader::Result GifSpriteLoader::ReadLoad(const LoadContext& context) {
  // Read the header
  std::array<std::uint8_t, 6> header{};
  if (const auto ec = context.stream.Read(header)) {
    return ec;
  }

  if (header != GIF87a && header != GIF89a) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  GifContext gif_context;
  gif_context.version = header == GIF89a ? 9 : 7;
  gif_context.lsd = ReadLogicalScreenDescriptor(context.stream);

  /* Is there a palette present? if so, read it */
  if (gif_context.lsd.fields.globalColorTableFlag) {
    const auto numOfColors = 2 << gif_context.lsd.fields.sizeOfGlobalColorTable;
    if (const auto ec = ReadPalette(context.stream, numOfColors, gif_context.globalPalette)) {
      return ec;
    }
  }

  /* Validate */
  if (gif_context.lsd.width > std::numeric_limits<uint16_t>::max() ||
      gif_context.lsd.height > std::numeric_limits<uint16_t>::max()) {
    return std::make_error_code(std::errc::invalid_argument);
  }

  /* The final sprite */
  auto finalSprite = Sprite::Create(
      Vec2D<uint16_t>(gif_context.lsd.width, gif_context.lsd.height),
      DrawableSurface::BitDepth::DEPTH_8,
      gif_context.globalPalette);

  /* Read data */
  while (true) {
    uint8_t block_type = 0;
    if (const auto ec = context.stream.Read(block_type)) {
      return ec;
    }

    /* Terminator */
    if (block_type == ';') {
      break;
    }

    /* Extension */
    if (block_type == '!') {
      if (const auto ec = ReadGifExtensionBlock(context.stream, gif_context)) {
        return ec;
      }
      continue;
    }

    /* Image Descriptor */
    if (block_type == ',') {
      if (const auto ec = ReadImage(context.stream, gif_context, finalSprite)) {
        return ec;
      }
    }
  }

  return {std::move(finalSprite)};
}
}// namespace e00::impl
