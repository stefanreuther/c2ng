/**
  *  \file u/t_gfx_save.cpp
  *  \brief Test for gfx::Save
  */

#include "gfx/save.hpp"

#include "t_gfx.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/base/staticassert.hpp"

/** Test saving, with emphasis on "what we save does not match the on-disk format".
    - use a palettized pixmap
    - make the size odd so the file will be padded */
void
TestGfxSave::testUnaligned()
{
    // Create a palettized pixmap.
    // Make it 3x5 pixels.
    afl::base::Ref<gfx::Canvas> can = gfx::PalettizedPixmap::create(3, 5)->makeCanvas();

    // Populate the pixmap
    const size_t NUM_COLORS = 3;
    static const gfx::ColorQuad_t palette[NUM_COLORS] = {
        COLORQUAD_FROM_RGB(32, 10, 10),
        COLORQUAD_FROM_RGB(10, 48, 10),
        COLORQUAD_FROM_RGB(10, 10, 64)
    };
    gfx::Color_t colors[NUM_COLORS];
    can->setPalette(0, palette, colors);

    // Verify that we got a palettized pixmap
    TS_ASSERT_EQUALS(colors[0], 0U);
    TS_ASSERT_EQUALS(colors[1], 1U);
    TS_ASSERT_EQUALS(colors[2], 2U);

    // Draw some pixels
    can->drawBar(gfx::Rectangle(0, 0, 100, 100), 0, 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    can->drawPixel(gfx::Point(1, 1), 1, gfx::OPAQUE_ALPHA);
    can->drawPixel(gfx::Point(1, 3), 2, gfx::OPAQUE_ALPHA);

    // Save it
    afl::io::InternalStream result;
    saveCanvas(*can, result);

    // Verify result
    // Each line is 12 bytes x 5 = 60
    static const uint8_t EXPECTED[] = {
        'B','M',                // signature
        54+60, 0, 0, 0,         // file size
        0, 0, 0, 0,             // reserved
        54, 0, 0, 0,            // header size
        40, 0, 0, 0,            // header size
        3, 0, 0, 0,             // width
        5, 0, 0, 0,             // height
        1, 0,                   // planes
        24, 0,                  // bpp
        0, 0, 0, 0,             // compression
        60, 0, 0, 0,            // pixmap size
        0, 0, 0, 0, 0, 0, 0, 0, // resolutions
        0, 0, 0, 0, 0, 0, 0, 0, // colors

        10,10,32,10,10,32,10,10,32,0,0,0,       // bottom row of pixels. Note the padding and the BGR order.
        10,10,32,64,10,10,10,10,32,0,0,0,
        10,10,32,10,10,32,10,10,32,0,0,0,
        10,10,32,10,48,10,10,10,32,0,0,0,
        10,10,32,10,10,32,10,10,32,0,0,0,       // top row of pixels
    };

    // Verify size.
    static_assert(sizeof(EXPECTED) == 54+60, "sizeof EXPECTED");
    TS_ASSERT_EQUALS(result.getSize(), afl::io::Stream::FileSize_t(54+60));

    // Verify content using TS_ASSERT for diagnostics, using afl to exercise that.
    TS_ASSERT_SAME_DATA(result.getContent().unsafeData(), EXPECTED, 54+60);
    TS_ASSERT(result.getContent().equalContent(EXPECTED));
}
