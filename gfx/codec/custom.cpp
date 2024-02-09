/**
  *  \file gfx/codec/custom.cpp
  *  \brief Class gfx::codec::Custom
  */

#include "gfx/codec/custom.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/transformreaderstream.hpp"
#include "afl/string/translator.hpp"
#include "gfx/colorquantizer.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "util/runlengthcompress.hpp"
#include "util/runlengthexpandtransform.hpp"

namespace {
    /*
     *  File Format Definitions
     *
     *  We repeat the palette here (same as ui::STANDARD_COLORS).
     *  In theory, the UI can use a different palette if desired.
     *  However, the implicit palette is defined by the file formats.
     */
    const gfx::ColorQuad_t PALETTE[] = {
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 0
        COLORQUAD_FROM_RGB( 97,  97,  97),    // 1
        COLORQUAD_FROM_RGB(194, 194, 194),    // 2
        COLORQUAD_FROM_RGB( 97, 242,  97),    // 3
        COLORQUAD_FROM_RGB(255,   0,   0),    // 4
        COLORQUAD_FROM_RGB( 64, 129,  64),    // 5
        COLORQUAD_FROM_RGB( 97,  97, 194),    // 6
        COLORQUAD_FROM_RGB(129, 129, 194),    // 7
        COLORQUAD_FROM_RGB( 97,  97, 129),    // 8
        COLORQUAD_FROM_RGB(255, 255,   0),    // 9
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 10 -- dynamic in 4-bpp
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 11 -- dynamic in 4-bpp
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 12 -- dynamic in 4-bpp
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 13 -- dynamic in 4-bpp
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 14 -- dynamic in 4-bpp
        COLORQUAD_FROM_RGB(255, 255, 255),    // 15
        // PCC1 repeats the first 16 entries 3 times, to place the next item at index 48
        COLORQUAD_FROM_RGB( 12,  12,  12),    // 16 -- 48
        COLORQUAD_FROM_RGB( 28,  28,  28),    // 17
        COLORQUAD_FROM_RGB( 44,  44,  44),    // 18
        COLORQUAD_FROM_RGB( 60,  60,  60),    // 19
        COLORQUAD_FROM_RGB( 76,  76,  76),    // 20
        COLORQUAD_FROM_RGB( 93,  93,  93),    // 21
        COLORQUAD_FROM_RGB(109, 109, 109),    // 22
        COLORQUAD_FROM_RGB(125, 125, 125),    // 23
        COLORQUAD_FROM_RGB(141, 141, 141),    // 24
        COLORQUAD_FROM_RGB(157, 157, 157),    // 25
        COLORQUAD_FROM_RGB(174, 174, 174),    // 26
        COLORQUAD_FROM_RGB(190, 190, 190),    // 27
        COLORQUAD_FROM_RGB(206, 206, 206),    // 28
        COLORQUAD_FROM_RGB(222, 222, 222),    // 29
        COLORQUAD_FROM_RGB(238, 238, 238),    // 30
        COLORQUAD_FROM_RGB(255, 255, 255),    // 31
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 32
        COLORQUAD_FROM_RGB( 24,   8,   0),    // 33
        COLORQUAD_FROM_RGB( 48,  16,   0),    // 34
        COLORQUAD_FROM_RGB( 72,  24,   0),    // 35
        COLORQUAD_FROM_RGB( 97,  32,   0),    // 36
        COLORQUAD_FROM_RGB(121,  40,   0),    // 37
        COLORQUAD_FROM_RGB(145,  48,   0),    // 38
        COLORQUAD_FROM_RGB(170,  56,   0),    // 39
        COLORQUAD_FROM_RGB(194,  64,   0),    // 40
        COLORQUAD_FROM_RGB(218,  72,   0),    // 41
        COLORQUAD_FROM_RGB(242,  80,   0),    // 42
        COLORQUAD_FROM_RGB(255,  85,   0),    // 43
        COLORQUAD_FROM_RGB(255,  97,   0),    // 44
        COLORQUAD_FROM_RGB(255, 109,   0),    // 45
        COLORQUAD_FROM_RGB(255, 121,   0),    // 46
        COLORQUAD_FROM_RGB(255, 133,   0),    // 47
        COLORQUAD_FROM_RGB(255, 145,   0),    // 48
        COLORQUAD_FROM_RGB(255, 157,   0),    // 49
        COLORQUAD_FROM_RGB(255, 170,   0),    // 50
        COLORQUAD_FROM_RGB(255, 182,   0),    // 51
        COLORQUAD_FROM_RGB(255, 194,   0),    // 52
        COLORQUAD_FROM_RGB(255, 206,   0),    // 53
        COLORQUAD_FROM_RGB(255, 218,   0),    // 54
        COLORQUAD_FROM_RGB(255, 230,   0),    // 55
        COLORQUAD_FROM_RGB(255, 242,   0),    // 56
        COLORQUAD_FROM_RGB(255, 255,   0),    // 57
        COLORQUAD_FROM_RGB(255, 255,  32),    // 58
        COLORQUAD_FROM_RGB(255, 255,  72),    // 59
        COLORQUAD_FROM_RGB(255, 255, 113),    // 60
        COLORQUAD_FROM_RGB(255, 255, 153),    // 61
        COLORQUAD_FROM_RGB(255, 255, 194),    // 62
        COLORQUAD_FROM_RGB(255, 255, 234),    // 63
        COLORQUAD_FROM_RGB(  0,   0,   0),    // 64
        COLORQUAD_FROM_RGB(  0,   0,  28),    // 65
        COLORQUAD_FROM_RGB(  0,   0,  56),    // 66
        COLORQUAD_FROM_RGB(  0,   0,  85),    // 67
        COLORQUAD_FROM_RGB( 32,  32, 121),    // 68
        COLORQUAD_FROM_RGB( 52,  52, 133),    // 69
        COLORQUAD_FROM_RGB( 72,  72, 145),    // 70
        COLORQUAD_FROM_RGB( 93,  93, 157),    // 71
        COLORQUAD_FROM_RGB(113, 113, 170),    // 72
        COLORQUAD_FROM_RGB(133, 133, 182),    // 73
        COLORQUAD_FROM_RGB(153, 153, 194),    // 74
        COLORQUAD_FROM_RGB(174, 174, 206),    // 75
        COLORQUAD_FROM_RGB(194, 194, 218),    // 76
        COLORQUAD_FROM_RGB(214, 214, 230),    // 77
        COLORQUAD_FROM_RGB(234, 234, 242),    // 78
        COLORQUAD_FROM_RGB(255, 255, 255),    // 79
        COLORQUAD_FROM_RGB(255,  12,   0),    // 80
        COLORQUAD_FROM_RGB(238,  28,   0),    // 81
        COLORQUAD_FROM_RGB(222,  44,   0),    // 82
        COLORQUAD_FROM_RGB(206,  60,   0),    // 83
        COLORQUAD_FROM_RGB(190,  76,   0),    // 84
        COLORQUAD_FROM_RGB(174,  93,   0),    // 85
        COLORQUAD_FROM_RGB(157, 109,   0),    // 86
        COLORQUAD_FROM_RGB(141, 125,   0),    // 87
        COLORQUAD_FROM_RGB(125, 141,   0),    // 88
        COLORQUAD_FROM_RGB(109, 157,   0),    // 89
        COLORQUAD_FROM_RGB( 93, 174,   0),    // 90
        COLORQUAD_FROM_RGB( 76, 190,   0),    // 91
        COLORQUAD_FROM_RGB( 60, 206,   0),    // 92
        COLORQUAD_FROM_RGB( 44, 222,   0),    // 93
        COLORQUAD_FROM_RGB( 28, 238,   0),    // 94
        COLORQUAD_FROM_RGB( 12, 255,   0),    // 95
        COLORQUAD_FROM_RGB(149, 149, 202),    // 96
        COLORQUAD_FROM_RGB(  0,   0, 170),    // 97
        COLORQUAD_FROM_RGB( 85,  85, 255),    // 98
        COLORQUAD_FROM_RGB(  0, 170,   0),    // 99
        COLORQUAD_FROM_RGB( 85, 255,  85),    // 100
        COLORQUAD_FROM_RGB(  0, 170, 170),    // 101
        COLORQUAD_FROM_RGB( 85, 255, 255),    // 102
        COLORQUAD_FROM_RGB(170,   0,   0),    // 103
        COLORQUAD_FROM_RGB(255,  85,  85),    // 104
        COLORQUAD_FROM_RGB(170,   0, 170),    // 105
        COLORQUAD_FROM_RGB(255,  85, 255),    // 106
        COLORQUAD_FROM_RGB(170, 170,   0),    // 107
        COLORQUAD_FROM_RGB(255, 255,  85),    // 108
        COLORQUAD_FROM_RGB(125,  97,   0),    // 109
        COLORQUAD_FROM_RGB(194,  97,   0),    // 110
        COLORQUAD_FROM_RGB(194,  97, 121),    // 111
        COLORQUAD_FROM_RGB(230, 137, 137),    // 112
        COLORQUAD_FROM_RGB(255, 121,   0),    // 113
        COLORQUAD_FROM_RGB(255, 194,   0),    // 114
        COLORQUAD_FROM_RGB(129,  64,  97),    // 115
        COLORQUAD_FROM_RGB(194,  97, 255),    // 116
        COLORQUAD_FROM_RGB( 68, 141,  68),    // 117
        COLORQUAD_FROM_RGB( 72, 157,  72),    // 118
        COLORQUAD_FROM_RGB( 76, 170,  76),    // 119
        COLORQUAD_FROM_RGB( 80, 186,  80),    // 120
        COLORQUAD_FROM_RGB( 85, 202,  85),    // 121
        COLORQUAD_FROM_RGB( 89, 214,  89),    // 122
        COLORQUAD_FROM_RGB( 93,   0,  93),    // 123
        COLORQUAD_FROM_RGB( 72,  72,   0),    // 124
        COLORQUAD_FROM_RGB( 48,  48,   0),    // 125
        COLORQUAD_FROM_RGB( 28,  28,   0),    // 126
        COLORQUAD_FROM_RGB(109,  72,  72),    // 127
        COLORQUAD_FROM_RGB(  0,  12,   0),    // 128
        COLORQUAD_FROM_RGB(  0,  28,   0),    // 129
        COLORQUAD_FROM_RGB(  0,  44,   0),    // 130
        COLORQUAD_FROM_RGB(  0,  60,   0),    // 131
        COLORQUAD_FROM_RGB(  0,  76,   0),    // 132
        COLORQUAD_FROM_RGB(  0,  93,   0),    // 133
        COLORQUAD_FROM_RGB(  0, 109,   0),    // 134
        COLORQUAD_FROM_RGB(  0, 125,   0),    // 135
        COLORQUAD_FROM_RGB(  0, 141,   0),    // 136
        COLORQUAD_FROM_RGB(  0, 157,   0),    // 137
        COLORQUAD_FROM_RGB(  0, 174,   0),    // 138
        COLORQUAD_FROM_RGB(  0, 190,   0),    // 139
        COLORQUAD_FROM_RGB(  0, 206,   0),    // 140
        COLORQUAD_FROM_RGB(  0, 222,   0),    // 141
        COLORQUAD_FROM_RGB(  0, 238,   0),    // 142
        COLORQUAD_FROM_RGB(  0, 255,   0),    // 143
        COLORQUAD_FROM_RGB( 12,  12,   0),    // 144
        COLORQUAD_FROM_RGB( 28,  28,   0),    // 145
        COLORQUAD_FROM_RGB( 44,  44,   0),    // 146
        COLORQUAD_FROM_RGB( 60,  60,   0),    // 147
        COLORQUAD_FROM_RGB( 76,  76,   0),    // 148
        COLORQUAD_FROM_RGB( 93,  93,   0),    // 149
        COLORQUAD_FROM_RGB(109, 109,   0),    // 150
        COLORQUAD_FROM_RGB(125, 125,   0),    // 151 -- 183
    };

    /** PCC1 4-bpp file format.
        - 2 bytes magic ('CC')
        - 2 bytes width
        - 2 bytes height
        - 5x3 bytes palette (slots 10..14, entry ranges 0..63)
        - pixel data */
    struct CCHeader {
        afl::bits::Value<afl::bits::Int16LE> width;
        afl::bits::Value<afl::bits::Int16LE> height;
        uint8_t palette[5][3];
    };

    /** PCC1 8-bpp file format.
        - 2 bytes magic ('CD')
        - 2 bytes width
        - 2 bytes height
        - 64x3 bytes palette (slots 192..255, ranges 0..63)
        - pixel data */
    struct CDHeader {
        afl::bits::Value<afl::bits::Int16LE> width;
        afl::bits::Value<afl::bits::Int16LE> height;
        uint8_t palette[64][3];
    };

    /** 8-bpp-with-transparency.
        - 2 bytes magic (0, 8)
        - 2 bytes width
        - 2 bytes height
        - pixel data. FF means transparent */
    struct GFXHeader {
        afl::bits::Value<afl::bits::Int16LE> width;
        afl::bits::Value<afl::bits::Int16LE> height;
    };

    /** Convert PCC v1 color to equivalent PCC v2 color. */
    inline uint8_t convertColor(uint8_t c)
    {
        if (c >= 0x30)
            return uint8_t(c - 0x20);
        else
            return uint8_t(c & 0x0F);
    }

    inline uint8_t convertColor16(uint8_t c)
    {
        if (c >= 10 && c < 15)
            return uint8_t(c-10 + 160);
        else
            return c;
    }

    /*
     *  Loaders
     */

    /** Load .cc image. Those are used by PCC v1 in 16 color mode.
        \pre magic number has been read */
    afl::base::Ptr<gfx::Canvas> loadCCImage(afl::io::Stream& s)
    {
        // Check header
        CCHeader header;
        if (s.read(afl::base::fromObject(header)) != sizeof(header)) {
            return 0;
        }

        int width = header.width;
        int height = header.height;
        if (width <= 0 || height <= 0 || width > 4000 || height > 4000) {
            // c2ng change: PCC2 only accepts up to 1 Mpx
            return 0;
        }

        // Temporary buffer
        afl::base::GrowableMemory<uint8_t> line;
        line.resize((width + 1)/2);

        // Load image
        afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(width, height);
        for (int i = 0; i < height; ++i) {
            // Read line
            s.read(line);

            // Unpack line
            afl::base::Memory<const uint8_t> in = line;
            afl::base::Memory<uint8_t> out = pix->row(i);
            while (const uint8_t* pin = in.eat()) {
                uint8_t value = *pin;
                if (uint8_t* pout = out.eat()) {
                    *pout = convertColor16(value & 15);
                }
                if (uint8_t* pout = out.eat()) {
                    *pout = convertColor16((value >> 4) & 15);
                }
            }
        }

        // Set up palette
        pix->setPalette(0, PALETTE);
        for (int i = 0; i < 5; ++i) {
            pix->setPalette(uint8_t(160+i), COLORQUAD_FROM_RGBA(header.palette[i][0] * 255/63,
                                                                header.palette[i][1] * 255/63,
                                                                header.palette[i][2] * 255/63,
                                                                gfx::OPAQUE_ALPHA));
        }

        return pix->makeCanvas().asPtr();
    }

    /** Load .cc image. Those are used by PCC v1 in 256 color mode.
        \pre magic number has been read. */
    afl::base::Ptr<gfx::Canvas> loadCDImage(afl::io::Stream& s)
    {
        // Check header
        CDHeader header;
        if (s.read(afl::base::fromObject(header)) != sizeof(header)) {
            return 0;
        }

        int width = header.width;
        int height = header.height;
        if (width <= 0 || height <= 0 || width > 4000 || height > 4000) {
            // c2ng change: PCC2 only accepts up to 1 Mpx
            return 0;
        }

        // Load image
        afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(width, height);
        s.fullRead(pix->pixels());

        // Convert pixel data
        afl::base::Memory<uint8_t> pixels = pix->pixels();
        while (uint8_t* p = pixels.eat()) {
            *p = convertColor(*p);
        }

        // Set up palette
        pix->setPalette(0, PALETTE);
        for (int i = 0; i < 64; ++i) {
            pix->setPalette(uint8_t(160+i), COLORQUAD_FROM_RGBA(header.palette[i][0] * 255/63,
                                                                header.palette[i][1] * 255/63,
                                                                header.palette[i][2] * 255/63,
                                                                gfx::OPAQUE_ALPHA));
        }

        return pix->makeCanvas().asPtr();
    }

    /** Load .gfx image. These are used for various things in PCC v1, and are
        the native format of the graphics library PCC v1 is based on. Unlike
        the other formats, we do not translate those; we assume they have the
        right palette. */
    afl::base::Ptr<gfx::Canvas> loadGFXImage(afl::io::Stream& s)
    {
        // Check header
        GFXHeader header;
        if (s.read(afl::base::fromObject(header)) != sizeof(header)) {
            return 0;
        }

        int width = header.width;
        int height = header.height;
        if (width <= 0 || height <= 0 || width > 4000 || height > 4000) {
            // c2ng change: PCC1.x only accepts up to 1 Mpx
            return 0;
        }

        // Load image
        afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(width, height);
        s.fullRead(pix->pixels());

        // Set up palette
        pix->setPalette(0, PALETTE);
        pix->setPalette(255, COLORQUAD_FROM_RGBA(0,0,0,gfx::TRANSPARENT_ALPHA));

        return pix->makeCanvas().asPtr();
    }

    afl::base::Ptr<gfx::Canvas> loadImageInternal(afl::io::Stream& in)
    {
        // ex ui/pixmap.cc:loadImageInternal, sort-of
        uint8_t magic[2];
        if (in.read(magic) == sizeof(magic)) {
            if (magic[0] == 'C' && magic[1] == 'C') {
                return loadCCImage(in);
            } else if (magic[0] == 'C' && magic[1] == 'D') {
                return loadCDImage(in);
            } else if (magic[0] == 0 && magic[1] == 8) {
                return loadGFXImage(in);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }

    /*
     *  Writers
     */

    void saveFourBitImage(gfx::Canvas& can, afl::io::Stream& out)
    {
        afl::base::Ref<gfx::PalettizedPixmap> encodedImage = gfx::ColorQuantizer()
            .setPalette(0, afl::base::Memory<const gfx::ColorQuad_t>(PALETTE).subrange(0, 16))
            .setUsablePaletteRange(0, 16)
            .setDynamicPaletteRange(10, 5)
            .quantize(can);

        gfx::ColorQuad_t customPalette[5];
        encodedImage->getPalette(10, customPalette);

        const int width = encodedImage->getWidth();
        const int height = encodedImage->getHeight();
        uint8_t signature[] = {'C','C'};
        CCHeader h;
        h.width = int16_t(width);
        h.height = int16_t(height);
        for (int i = 0; i < 5; ++i) {
            h.palette[i][0] =   RED_FROM_COLORQUAD(customPalette[i]) / 4;
            h.palette[i][1] = GREEN_FROM_COLORQUAD(customPalette[i]) / 4;
            h.palette[i][2] =  BLUE_FROM_COLORQUAD(customPalette[i]) / 4;
        }

        out.fullWrite(signature);
        out.fullWrite(afl::base::fromObject(h));

        std::vector<uint8_t> rowBuffer;
        rowBuffer.reserve((width + 1) / 2);
        for (int i = 0, n = height; i < n; ++i) {
            rowBuffer.clear();

            afl::base::ConstBytes_t bytes = encodedImage->row(i);
            while (const uint8_t* p1 = bytes.eat()) {
                if (const uint8_t* p2 = bytes.eat()) {
                    rowBuffer.push_back(static_cast<uint8_t>(*p1 + 16 * *p2));
                } else {
                    rowBuffer.push_back(*p1);
                }
            }
            out.fullWrite(rowBuffer);
        }
    }

    void saveEightBitImage(gfx::Canvas& can, afl::io::Stream& out)
    {
        afl::base::Memory<const gfx::ColorQuad_t> pal(PALETTE);
        afl::base::Ref<gfx::PalettizedPixmap> encodedImage = gfx::ColorQuantizer()
            .setPalette(0,  pal.subrange(0, 16))
            .setPalette(48, pal.subrange(16))
            .setUsablePaletteRange(0, 256)
            .setDynamicPaletteRange(192, 64)
            .quantize(can);

        gfx::ColorQuad_t customPalette[64];
        encodedImage->getPalette(192, customPalette);

        uint8_t signature[] = {'C','D'};
        CDHeader h;
        h.width = int16_t(encodedImage->getWidth());
        h.height = int16_t(encodedImage->getHeight());
        for (int i = 0; i < 64; ++i) {
            h.palette[i][0] =   RED_FROM_COLORQUAD(customPalette[i]) / 4;
            h.palette[i][1] = GREEN_FROM_COLORQUAD(customPalette[i]) / 4;
            h.palette[i][2] =  BLUE_FROM_COLORQUAD(customPalette[i]) / 4;
        }

        out.fullWrite(signature);
        out.fullWrite(afl::base::fromObject(h));
        out.fullWrite(encodedImage->pixels());
    }

    void saveImageInternal(gfx::codec::Custom::Mode mode, gfx::Canvas& can, afl::io::Stream& out)
    {
        switch (mode) {
         case gfx::codec::Custom::FourBit:
            saveFourBitImage(can, out);
            break;
         case gfx::codec::Custom::EightBit:
            saveEightBitImage(can, out);
            break;
        }
    }
}

void
gfx::codec::Custom::save(Canvas& can, afl::io::Stream& stream)
{
    if (m_compressed) {
        afl::io::InternalStream uncompressedData;
        saveImageInternal(m_mode, can, uncompressedData);

        afl::base::GrowableBytes_t compressedData;
        util::encodeRLE(compressedData, uncompressedData.getContent());
        stream.fullWrite(compressedData);
    } else {
        saveImageInternal(m_mode, can, stream);
    }
}

afl::base::Ref<gfx::Canvas>
gfx::codec::Custom::load(afl::io::Stream& stream)
{
    // Load as uncompressed image
    stream.setPos(0);
    afl::base::Ptr<gfx::Canvas> result = loadImageInternal(stream);
    if (result.get() == 0) {
        // Try again compressed
        stream.setPos(0);
        util::RunLengthExpandTransform tx;
        afl::io::TransformReaderStream decompReader(stream, tx);
        result = loadImageInternal(decompReader);
    }
    if (result.get() == 0) {
        throw afl::except::FileFormatException(stream, AFL_TRANSLATE_STRING("Invalid file format"));
    }
    return *result;
}
