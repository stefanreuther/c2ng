/**
  *  \file gfx/codec/bmp.cpp
  *  \brief Class gfx::codec::BMP
  */

#include "gfx/codec/bmp.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/rgbapixmap.hpp"

using afl::base::GrowableMemory;
using afl::base::Ref;

namespace {
    /* Shortcuts */
    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;

    /* BMP file header. */
    struct Header {
        uint8_t signature[2];             // 'BM'
        UInt32_t fileSize;                // size of whole file
        UInt32_t reserved;                // 0
        UInt32_t pixelOffset;             // offset of pixel data (=size of this header)

        UInt32_t headerSize;              // size of the following data (=40)
        UInt32_t width;
        UInt32_t height;
        UInt16_t planes;                  // set to 1
        UInt16_t bits;                    // number of bits per pixel
        UInt32_t compression;             // compression method (0=uncompressed)
        UInt32_t pixmapSize;              // size of pixel data
        UInt32_t horizontalResolution;    // horizontal resolution in ppm (0=unspecified)
        UInt32_t verticalResolution;
        UInt32_t numColors;               // number of colors (0=truecolor, don't care)
        UInt32_t numImportantColors;      // number of important colors (0=don't care)
    };
    static_assert(sizeof(Header) == 54, "sizeof Header");

    /* Load palettized (8-bpp) image */
    Ref<gfx::Canvas> loadPalettizedPixmap(const Header& h, afl::io::Stream& stream)
    {
        const int width = int(h.width);
        const int height = int(h.height);
        Ref<gfx::PalettizedPixmap> result = gfx::PalettizedPixmap::create(width, height);

        // Palette
        {
            // Validate
            // The numColors field is set for many, but not all BMP files.
            // Thus, check just how much data there is between the header and the pixmap data.
            const uint32_t paletteBytes = h.pixelOffset - uint32_t(h.headerSize + 14);
            const uint32_t paletteEntries = std::min<uint32_t>(paletteBytes/4, 256);
            if (paletteEntries == 0 || paletteBytes > 2000) {
                throw afl::except::FileFormatException(stream, "Unsupported file");
            }

            // Read
            GrowableMemory<uint8_t> paletteBuffer;
            paletteBuffer.resize(paletteBytes);
            stream.fullRead(paletteBuffer);

            // Copy to result
            for (size_t i = 0; i < paletteEntries; ++i) {
                result->setPalette(uint8_t(i), COLORQUAD_FROM_RGB(*paletteBuffer.at(4*i+2), *paletteBuffer.at(4*i+1), *paletteBuffer.at(4*i+0)));
            }
        }

        // Image
        {
            GrowableMemory<uint8_t> rowBuffer;
            rowBuffer.resize((width + 3) & ~3);
            for (int i = 0; i < height; ++i) {
                stream.fullRead(rowBuffer);
                result->row(height-i-1).copyFrom(rowBuffer);
            }
        }

        return result->makeCanvas();
    }

    /* Load RGB (24-bpp) palettized image */
    Ref<gfx::Canvas> loadRGBPixmap(const Header& h, afl::io::Stream& stream)
    {
        const int width = int(h.width);
        const int height = int(h.height);
        Ref<gfx::RGBAPixmap> result = gfx::RGBAPixmap::create(width, height);

        // Image
        GrowableMemory<uint8_t> rowBuffer;
        rowBuffer.resize((3*width + 3) & ~3);
        for (int y = 0; y < height; ++y) {
            stream.fullRead(rowBuffer);
            for (int x = 0; x < width; ++x) {
                *result->row(height-y-1).at(x) = COLORQUAD_FROM_RGB(*rowBuffer.at(3*x+2), *rowBuffer.at(3*x+1), *rowBuffer.at(3*x+0));
            }
        }

        return result->makeCanvas();
    }

}

// Save canvas to file in 24-bit BMP format.
void
gfx::codec::BMP::save(Canvas& can, afl::io::Stream& stream)
{
    // Get dimensions
    Point size = can.getSize();
    uint32_t pixmapSize = ((3 * size.getX() + 3) & ~3) * size.getY();

    // Prepare header
    Header h;
    h.signature[0]         = 'B';
    h.signature[1]         = 'M';
    h.fileSize             = uint32_t(pixmapSize + sizeof(h));
    h.reserved             = 0;
    h.pixelOffset          = sizeof(h);
    h.headerSize           = 40;
    h.width                = size.getX();
    h.height               = size.getY();
    h.planes               = 1;
    h.bits                 = 24;
    h.compression          = 0;
    h.pixmapSize           = pixmapSize;
    h.horizontalResolution = 0;
    h.verticalResolution   = 0;
    h.numColors            = 0;
    h.numImportantColors   = 0;
    stream.fullWrite(afl::base::fromObject(h));

    // Write pixels. Note that *.bmp is stored upside-down.
    for (int y = size.getY(); y > 0; --y) {
        const int STEP = 256;
        for (int x = 0; x < size.getX(); x += STEP) {
            int amount = std::min(STEP, size.getX() - x);

            // Read raw colors
            Color_t rawColors[STEP];
            afl::base::Memory<Color_t> rawColorMem(rawColors);
            rawColorMem.trim(amount);
            can.getPixels(Point(x, y-1), rawColorMem);

            // Decode the colors
            ColorQuad_t decodedColors[STEP];
            afl::base::Memory<ColorQuad_t> decodedColorMem(decodedColors);
            decodedColorMem.trim(amount);
            can.decodeColors(rawColorMem, decodedColorMem);

            // Pack into triplets
            uint8_t packedColors[STEP*3];
            size_t index = 0;
            for (int i = 0; i < amount; ++i) {
                packedColors[index++] = BLUE_FROM_COLORQUAD(decodedColors[i]);
                packedColors[index++] = GREEN_FROM_COLORQUAD(decodedColors[i]);
                packedColors[index++] = RED_FROM_COLORQUAD(decodedColors[i]);
            }

            // The file format requires that row size is multiple of 4
            static_assert(sizeof(packedColors) % 4 == 0, "sizeof packedColors");
            while ((index % 4) != 0) {
                packedColors[index++] = 0;
            }

            // Write
            stream.fullWrite(afl::base::ConstBytes_t(packedColors).trim(index));
        }
    }
}

afl::base::Ref<gfx::Canvas>
gfx::codec::BMP::load(afl::io::Stream& stream)
{
    // Read header
    Header h;
    stream.fullRead(afl::base::fromObject(h));

    // Validate
    if (h.signature[0] != 'B'
        || h.signature[1] != 'M'
        || h.headerSize < 40
        || h.width > 10000
        || h.height > 10000
        || h.planes != 1
        || h.compression != 0)
    {
        throw afl::except::FileFormatException(stream, "Unsupported file");
    }

    // Skip if header is extra large
    // (ImageMagick does that; 'file' reports it as "Windows 98/2000 and newer format")
    if (h.headerSize != 40) {
        stream.setPos(stream.getPos() + (h.headerSize - 40));
    }

    // Decode depending on bits
    if (h.bits == 8) {
        return loadPalettizedPixmap(h, stream);
    } else if (h.bits == 24) {
        return loadRGBPixmap(h, stream);
    } else {
        throw afl::except::FileFormatException(stream, "Unsupported file");
    }
}
