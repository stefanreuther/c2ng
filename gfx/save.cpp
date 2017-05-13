/**
  *  \file gfx/save.cpp
  *  \brief Function gfx::saveCanvas
  */

#include "gfx/save.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "gfx/canvas.hpp"

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
}

// Save canvas to file in 24-bit BMP format.
void
gfx::saveCanvas(Canvas& can, afl::io::Stream& stream)
{
    // Get dimensions
    Point size = can.getSize();
    uint32_t pixmapSize = ((size.getX() + 3) & ~3) * size.getY() * 3;

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
