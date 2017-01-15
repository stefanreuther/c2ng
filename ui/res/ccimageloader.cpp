/**
  *  \file ui/res/ccimageloader.cpp
  */

#include "ui/res/ccimageloader.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/int16le.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/colorscheme.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/io/transformreaderstream.hpp"
#include "util/runlengthexpandtransform.hpp"

namespace {
    /** Convert PCC v1 color to equivalent PCC v2 color. */
    inline uint8_t convertColor(uint8_t c)
    {
        if (c >= 0x30)
            return c - 0x20;
        else
            return c & 0x0F;
    }

    inline uint8_t convertColor16(uint8_t c)
    {
        if (c >= 10 && c < 15)
            return c-10 + 160;
        else
            return c;
    }

    /** Load .cc image. Those are used by PCC v1 in 16 color mode.
        \pre magic number has been read.
        \assumes stream need not be seekable.

        File format:
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
            // c2ng change: PCC1.x only accepts up to 1 Mpx
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
                    *pout = convertColor16((value >> 4) & 15);
                }
                if (uint8_t* pout = out.eat()) {
                    *pout = convertColor16(value & 15);
                }
            }
        }

        // Set up palette
        pix->setPalette(0, ui::STANDARD_COLORS);
        for (int i = 0; i < 5; ++i) {
            pix->setPalette(160+i, COLORQUAD_FROM_RGBA(header.palette[i][0] * 255/63,
                                                       header.palette[i][1] * 255/63,
                                                       header.palette[i][2] * 255/63,
                                                       gfx::OPAQUE_ALPHA));
        }

        return pix->makeCanvas().asPtr();
    }

    /** Load .cc image. Those are used by PCC v1 in 256 color mode.
        \pre magic number has been read.
        \assumes stream need not be seekable.

        File format:
        - 2 bytes magic ('CD')
        - 2 bytes width
        - 2 bytes height
        - 64x3 bytes palette (slots 192..255, ranges 0..63)
        - pixel data

        Palette will be shuffled accordingly. */
    struct CDHeader {
        afl::bits::Value<afl::bits::Int16LE> width;
        afl::bits::Value<afl::bits::Int16LE> height;
        uint8_t palette[64][3];
    };

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
            // c2ng change: PCC1.x only accepts up to 1 Mpx
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
        pix->setPalette(0, ui::STANDARD_COLORS);
        for (int i = 0; i < 64; ++i) {
            pix->setPalette(160+i, COLORQUAD_FROM_RGBA(header.palette[i][0] * 255/63,
                                                       header.palette[i][1] * 255/63,
                                                       header.palette[i][2] * 255/63,
                                                       gfx::OPAQUE_ALPHA));
        }

        return pix->makeCanvas().asPtr();
    }


    struct GFXHeader {
        afl::bits::Value<afl::bits::Int16LE> width;
        afl::bits::Value<afl::bits::Int16LE> height;
    };

    /** Load .gfx image. These are used for various things in PCC v1, and are
        the native format of the graphics library PCC v1 is based on. Unlike
        the other formats, we do not translate those; we assume they have the
        right palette.

        File format:
        - 2 bytes magic (0, 8)
        - 2 bytes width
        - 2 bytes height
        - pixel data. FF means transparent */
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
        pix->setPalette(0, ui::STANDARD_COLORS);
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
}


ui::res::CCImageLoader::CCImageLoader()
{ }

afl::base::Ptr<gfx::Canvas>
ui::res::CCImageLoader::loadImage(afl::io::Stream& in)
{
    // Load as uncompressed image
    in.setPos(0);
    afl::base::Ptr<gfx::Canvas> result = loadImageInternal(in);
    if (result.get() == 0) {
        // Try again compressed
        in.setPos(0);
        util::RunLengthExpandTransform tx;
        afl::io::TransformReaderStream decompReader(in, tx);
        result = loadImageInternal(decompReader);
    }
    return result;
}
