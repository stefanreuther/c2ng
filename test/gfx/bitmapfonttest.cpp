/**
  *  \file test/gfx/bitmapfonttest.cpp
  *  \brief Test for gfx::BitmapFont
  */

#include "gfx/bitmapfont.hpp"

#include "afl/charset/utf8.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/bitmapglyph.hpp"
#include "gfx/palettizedpixmap.hpp"

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
AFL_TEST("gfx.BitmapFont:load", a)
{
    // Load from file
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(MIN_FONT_FILE);
    testee.load(ms, 0, tx);

    // Verify
    a.checkEqual("01. getHeight", testee.getHeight(), 4);
    a.checkEqual("02. getCurrentCharacterLimit", testee.getCurrentCharacterLimit(), 0xE131U);
    a.checkNull("03. getGlyph", testee.getGlyph(0));
    a.checkNonNull("04. getGlyph", testee.getGlyph('A'));
    a.checkNonNull("05. getGlyph", testee.getGlyph(0xE108));

    // Text output
    String_t s = "A";
    afl::charset::Utf8().append(s, afl::charset::makeErrorCharacter(0x80));
    s += 'B';
    afl::charset::Utf8().append(s, 0x8000);
    s += 'C';
    a.checkEqual("11. getTextWidth",  testee.getTextWidth(s), 17);
    a.checkEqual("12. getTextHeight", testee.getTextHeight(s), 4);

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
    a.checkEqualContent<uint8_t>("11. pixels", pix->pixels(), EXPECTED);
}

/** Test addNewGlyph. */
AFL_TEST("gfx.BitmapFont:addNewGlyph", a)
{
    // Empty font
    gfx::BitmapFont testee;

    // Initial state
    a.checkEqual("01. getTextWidth",  testee.getTextWidth("A"), 0);
    a.checkEqual("02. getTextHeight", testee.getTextHeight("A"), 0);
    a.checkEqual("03. getTextWidth",  testee.getTextWidth("ABC"), 0);

    // Add a single glyph
    testee.addNewGlyph('A', new gfx::BitmapGlyph(5, 3));
    a.checkEqual("11. getTextWidth",  testee.getTextWidth("ABC"), 5);
    a.checkEqual("12. getTextWidth",  testee.getTextWidth("A"), 5);
    a.checkEqual("13. getTextHeight", testee.getTextHeight("A"), 3);

    // Add more glyphs
    testee.addNewGlyph(0x401, new gfx::BitmapGlyph(6, 4));
    testee.addNewGlyph(0x400, new gfx::BitmapGlyph(3, 4));

    a.checkEqual("21. getTextWidth",  testee.getTextWidth("A"), 5);
    a.checkEqual("22. getTextHeight", testee.getTextHeight("A"), 4);          // got larger!
    a.checkEqual("23. getTextWidth",  testee.getTextWidth("\xD0\x81"), 6);
    a.checkEqual("24. getTextHeight", testee.getTextHeight("\xD0\x81"), 4);
    a.checkEqual("25. getTextWidth",  testee.getTextWidth("A\xD0\x81"), 11);

    // Remove a glyph
    testee.addNewGlyph('A', 0);
    a.checkEqual("31. getTextWidth",  testee.getTextWidth("ABC"), 0);
    a.checkEqual("32. getTextWidth",  testee.getTextWidth("A"), 0);
    a.checkEqual("33. getTextHeight", testee.getTextHeight("A"), 4);

    // Add replacement glyph
    testee.addNewGlyph(0xE100, new gfx::BitmapGlyph(2, 2));
    a.checkEqual("41. getTextWidth",  testee.getTextWidth("ABC"), 6);
    a.checkEqual("42. getTextWidth",  testee.getTextWidth("A"), 2);
    a.checkEqual("43. getTextHeight", testee.getTextHeight("A"), 4);

    // Adding null does not enlarge character limit
    a.checkEqual("51. getCurrentCharacterLimit", testee.getCurrentCharacterLimit(), 0xE101U);
    testee.addNewGlyph(0xF000, 0);
    a.checkEqual("52. getCurrentCharacterLimit", testee.getCurrentCharacterLimit(), 0xE101U);
}

/** Test invalid files. */

// File too short
AFL_TEST("gfx.BitmapFont:load:error:truncated", a)
{
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(afl::string::toBytes(""));
    AFL_CHECK_THROWS(a, testee.load(ms, 0, tx), afl::except::FileProblemException);
}

// Bad magic
AFL_TEST("gfx.BitmapFont:load:error:bad-magic", a)
{
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(afl::string::toBytes("abcxyz"));
    AFL_CHECK_THROWS(a, testee.load(ms, 0, tx), afl::except::FileProblemException);
}

// Font not found
AFL_TEST("gfx.BitmapFont:load:error:font-id-not-found", a)
{
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::io::ConstMemoryStream ms(MIN_FONT_FILE);
    AFL_CHECK_THROWS(a, testee.load(ms, 1, tx), afl::except::FileProblemException);
}

// File too short
AFL_TEST("gfx.BitmapFont:load:error:too-short", a)
{
    gfx::BitmapFont testee;
    afl::string::NullTranslator tx;
    afl::base::ConstBytes_t bytes(MIN_FONT_FILE);
    bytes.trim(bytes.size()-1);
    afl::io::ConstMemoryStream ms(bytes);
    AFL_CHECK_THROWS(a, testee.load(ms, 0, tx), afl::except::FileProblemException);
}
