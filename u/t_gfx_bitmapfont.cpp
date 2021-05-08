/**
  *  \file u/t_gfx_bitmapfont.cpp
  *  \brief Test for gfx::BitmapFont
  */

#include "gfx/bitmapfont.hpp"

#include "t_gfx.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/io/constmemorystream.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/bitmapglyph.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/nulltranslator.hpp"

#define TS_ASSERT_SAME(got, expected) \
    TS_ASSERT_EQUALS(got.size(), sizeof(expected)); \
    TS_ASSERT_SAME_DATA(got.unsafeData(), expected, sizeof(expected))

namespace {
    /* A font file saved with (modified) c2reshack.
       It has these characters: A,B,C, E100, E108, E110, E120, E130. */
    const uint8_t MIN_FONT_FILE[] = {
        0x46, 0x4e, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04,
        0x00, 0x08, 0x00, 0x53, 0x00, 0x00, 0x00, 0x41, 0x00, 0x04, 0x00, 0x59, 0x00, 0x00, 0x00, 0x42,
        0x00, 0x04, 0x00, 0x63, 0x00, 0x00, 0x00, 0x43, 0x00, 0x03, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00,
        0xe1, 0x03, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x08, 0xe1, 0x03, 0x00, 0x75, 0x00, 0x00, 0x00, 0x10,
        0xe1, 0x03, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x20, 0xe1, 0x03, 0x00, 0x81, 0x00, 0x00, 0x00, 0x30,
        0xe1, 0x03, 0x00, 0x40, 0xe0, 0xa0, 0x00, 0x00, 0x00, 0xe0, 0xc0, 0xe0, 0x00, 0x01, 0x00, 0x02,
        0x00, 0x01, 0x00, 0xc0, 0x80, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xc0,
        0x80, 0x00, 0x00, 0x00, 0x00, 0x20, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0x00,
        0x00, 0x00, 0x00, 0x60, 0x20, 0x00, 0x00
    };
}

/** Test file access. */
void
TestGfxBitmapFont::testFile()
{
    // Load from file
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(MIN_FONT_FILE);
    testee.load(ms, 0, tx);

    // Verify
    TS_ASSERT_EQUALS(testee.getHeight(), 4);
    TS_ASSERT_EQUALS(testee.getCurrentCharacterLimit(), 0xE131U);
    TS_ASSERT(testee.getGlyph(0) == 0);
    TS_ASSERT(testee.getGlyph('A') != 0);
    TS_ASSERT(testee.getGlyph(0xE108) != 0);

    // Text output
    String_t s = "A";
    afl::charset::Utf8().append(s, afl::charset::makeErrorCharacter(0x80));
    s += 'B';
    afl::charset::Utf8().append(s, 0x8000);
    s += 'C';
    TS_ASSERT_EQUALS(testee.getTextWidth(s), 17);
    TS_ASSERT_EQUALS(testee.getTextHeight(s), 4);
    
    // - make palettized pixmap with sensible palette
    afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(20, 6));
    for (int i = 0; i < 256; ++i) {
        pix->setPalette(uint8_t(i), COLORQUAD_FROM_RGBA(i, i, i, gfx::OPAQUE_ALPHA));
    }
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

    // - draw
    gfx::BaseContext ctx(*can);
    ctx.setRawColor(8);
    testee.outText(ctx, gfx::Point(1, 2), s);

    // - verify
    static const uint8_t EXPECTED[] = {
        //    A     E108/E130    B     E108/E110/E120/E130    C
        0, 0,0,0,0,   0,0,0,  0,0,0,0,       0,0,0,         0,0,0, 0,0,
        0, 0,0,0,0,   0,0,0,  0,0,0,0,       0,0,0,         0,0,0, 0,0,
        0, 0,8,0,0,   8,8,0,  8,8,8,0,       8,8,8,         8,8,0, 0,0,
        0, 8,8,8,0,   8,0,0,  8,8,4,0,       8,8,8,         8,0,0, 0,0,
        0, 8,0,8,0,   0,8,8,  8,8,8,0,       8,8,8,         8,8,0, 0,0,
        0, 0,0,0,0,   0,0,8,  0,0,0,0,       8,8,8,         0,0,0, 0,0,
    };
    TS_ASSERT_SAME(pix->pixels(), EXPECTED);
}

/** Test addNewGlyph. */
void
TestGfxBitmapFont::testAdd()
{
    // Empty font
    gfx::BitmapFont testee;

    // Initial state
    TS_ASSERT_EQUALS(testee.getTextWidth("A"), 0);
    TS_ASSERT_EQUALS(testee.getTextHeight("A"), 0);
    TS_ASSERT_EQUALS(testee.getTextWidth("ABC"), 0);

    // Add a single glyph
    testee.addNewGlyph('A', new gfx::BitmapGlyph(5, 3));
    TS_ASSERT_EQUALS(testee.getTextWidth("ABC"), 5);
    TS_ASSERT_EQUALS(testee.getTextWidth("A"), 5);
    TS_ASSERT_EQUALS(testee.getTextHeight("A"), 3);

    // Add more glyphs
    testee.addNewGlyph(0x401, new gfx::BitmapGlyph(6, 4));
    testee.addNewGlyph(0x400, new gfx::BitmapGlyph(3, 4));

    TS_ASSERT_EQUALS(testee.getTextWidth("A"), 5);
    TS_ASSERT_EQUALS(testee.getTextHeight("A"), 4);          // got larger!
    TS_ASSERT_EQUALS(testee.getTextWidth("\xD0\x81"), 6);
    TS_ASSERT_EQUALS(testee.getTextHeight("\xD0\x81"), 4);
    TS_ASSERT_EQUALS(testee.getTextWidth("A\xD0\x81"), 11);

    // Remove a glyph
    testee.addNewGlyph('A', 0);
    TS_ASSERT_EQUALS(testee.getTextWidth("ABC"), 0);
    TS_ASSERT_EQUALS(testee.getTextWidth("A"), 0);
    TS_ASSERT_EQUALS(testee.getTextHeight("A"), 4);

    // Add replacement glyph
    testee.addNewGlyph(0xE100, new gfx::BitmapGlyph(2, 2));
    TS_ASSERT_EQUALS(testee.getTextWidth("ABC"), 6);
    TS_ASSERT_EQUALS(testee.getTextWidth("A"), 2);
    TS_ASSERT_EQUALS(testee.getTextHeight("A"), 4);

    // Adding null does not enlarge character limit
    TS_ASSERT_EQUALS(testee.getCurrentCharacterLimit(), 0xE101U);
    testee.addNewGlyph(0xF000, 0);
    TS_ASSERT_EQUALS(testee.getCurrentCharacterLimit(), 0xE101U);
}

/** Test invalid files. */
void
TestGfxBitmapFont::testFileErrors()
{
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;

    // File too short
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(""));
        TS_ASSERT_THROWS(testee.load(ms, 0, tx), afl::except::FileProblemException);
    }

    // Bad magic
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("abcxyz"));
        TS_ASSERT_THROWS(testee.load(ms, 0, tx), afl::except::FileProblemException);
    }

    // Font not found
    {
        afl::io::ConstMemoryStream ms(MIN_FONT_FILE);
        TS_ASSERT_THROWS(testee.load(ms, 1, tx), afl::except::FileProblemException);
    }

    // File too short
    {
        afl::base::ConstBytes_t bytes(MIN_FONT_FILE);
        bytes.trim(bytes.size()-1);
        afl::io::ConstMemoryStream ms(bytes);
        TS_ASSERT_THROWS(testee.load(ms, 0, tx), afl::except::FileProblemException);
    }
}

