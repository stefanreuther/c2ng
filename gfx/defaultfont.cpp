/**
  *  \file gfx/defaultfont.cpp
  *  \brief Default Font
  *
  *  Providing a default font allows us to eliminate many "oops I don't have a font" codepaths,
  *  and costs only a little more than a kilobyte of program size.
  */

#include "gfx/defaultfont.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/bitmapglyph.hpp"
#include "afl/base/staticassert.hpp"

namespace {
    // 32 .. 126 (inclusive) = 95 characters
    const size_t NUM_CHARS = 95;

    // 8 bytes per character = 8 pixel height (width always is 8 bits)
    const size_t BYTES_PER_CHAR = 8;

    /** Font data.
        These are characters 32..127 of an 8x8 VGA screen font.
        These bits have been taken from "READABLE.F08" contained in an old archive fntcol15.zip,
        that reports this font to be public domain. */
    const uint8_t FONT_DATA[][BYTES_PER_CHAR] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 },
        { 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00 },
        { 0x18, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x18, 0x00 },
        { 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00 },
        { 0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00 },
        { 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00 },
        { 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00 },
        { 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00 },
        { 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 },
        { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 },
        { 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00 },
        { 0x7c, 0xce, 0xde, 0xf6, 0xe6, 0xc6, 0x7c, 0x00 },
        { 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00 },
        { 0x7c, 0xc6, 0x06, 0x7c, 0xc0, 0xc0, 0xfe, 0x00 },
        { 0xfc, 0x06, 0x06, 0x3c, 0x06, 0x06, 0xfc, 0x00 },
        { 0x0c, 0xcc, 0xcc, 0xcc, 0xfe, 0x0c, 0x0c, 0x00 },
        { 0xfe, 0xc0, 0xfc, 0x06, 0x06, 0xc6, 0x7c, 0x00 },
        { 0x7c, 0xc0, 0xc0, 0xfc, 0xc6, 0xc6, 0x7c, 0x00 },
        { 0xfe, 0x06, 0x06, 0x0c, 0x18, 0x30, 0x30, 0x00 },
        { 0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0x00 },
        { 0x7c, 0xc6, 0xc6, 0x7e, 0x06, 0x06, 0x7c, 0x00 },
        { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 },
        { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 },
        { 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00 },
        { 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00 },
        { 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00 },
        { 0x3c, 0x66, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00 },
        { 0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x7e, 0x00 },
        { 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0x00 },
        { 0xfc, 0xc6, 0xc6, 0xfc, 0xc6, 0xc6, 0xfc, 0x00 },
        { 0x7c, 0xc6, 0xc0, 0xc0, 0xc0, 0xc6, 0x7c, 0x00 },
        { 0xf8, 0xcc, 0xc6, 0xc6, 0xc6, 0xcc, 0xf8, 0x00 },
        { 0xfe, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xfe, 0x00 },
        { 0xfe, 0xc0, 0xc0, 0xf8, 0xc0, 0xc0, 0xc0, 0x00 },
        { 0x7c, 0xc6, 0xc0, 0xc0, 0xce, 0xc6, 0x7c, 0x00 },
        { 0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00 },
        { 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00 },
        { 0x06, 0x06, 0x06, 0x06, 0x06, 0xc6, 0x7c, 0x00 },
        { 0xc6, 0xcc, 0xd8, 0xf0, 0xd8, 0xcc, 0xc6, 0x00 },
        { 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0x00 },
        { 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00 },
        { 0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00 },
        { 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00 },
        { 0xfc, 0xc6, 0xc6, 0xfc, 0xc0, 0xc0, 0xc0, 0x00 },
        { 0x7c, 0xc6, 0xc6, 0xc6, 0xd6, 0xde, 0x7c, 0x06 },
        { 0xfc, 0xc6, 0xc6, 0xfc, 0xd8, 0xcc, 0xc6, 0x00 },
        { 0x7c, 0xc6, 0xc0, 0x7c, 0x06, 0xc6, 0x7c, 0x00 },
        { 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 },
        { 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfe, 0x00 },
        { 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x38, 0x00 },
        { 0xc6, 0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },
        { 0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00 },
        { 0xc6, 0xc6, 0xc6, 0x7c, 0x18, 0x30, 0xe0, 0x00 },
        { 0xfe, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xfe, 0x00 },
        { 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00 },
        { 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00 },
        { 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00 },
        { 0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff },
        { 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x7c, 0x06, 0x7e, 0xc6, 0x7e, 0x00 },
        { 0xc0, 0xc0, 0xc0, 0xfc, 0xc6, 0xc6, 0xfc, 0x00 },
        { 0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc6, 0x7c, 0x00 },
        { 0x06, 0x06, 0x06, 0x7e, 0xc6, 0xc6, 0x7e, 0x00 },
        { 0x00, 0x00, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00 },
        { 0x1c, 0x36, 0x30, 0x78, 0x30, 0x30, 0x78, 0x00 },
        { 0x00, 0x00, 0x7e, 0xc6, 0xc6, 0x7e, 0x06, 0xfc },
        { 0xc0, 0xc0, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0x00 },
        { 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00 },
        { 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0xc6, 0x7c },
        { 0xc0, 0xc0, 0xcc, 0xd8, 0xf8, 0xcc, 0xc6, 0x00 },
        { 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00 },
        { 0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xd6, 0x00 },
        { 0x00, 0x00, 0xfc, 0xc6, 0xc6, 0xc6, 0xc6, 0x00 },
        { 0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00 },
        { 0x00, 0x00, 0xfc, 0xc6, 0xc6, 0xfc, 0xc0, 0xc0 },
        { 0x00, 0x00, 0x7e, 0xc6, 0xc6, 0x7e, 0x06, 0x06 },
        { 0x00, 0x00, 0xfc, 0xc6, 0xc0, 0xc0, 0xc0, 0x00 },
        { 0x00, 0x00, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x00 },
        { 0x18, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0e, 0x00 },
        { 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0x7e, 0x00 },
        { 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x7c, 0x38, 0x00 },
        { 0x00, 0x00, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },
        { 0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00 },
        { 0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0xfc },
        { 0x00, 0x00, 0xfe, 0x0c, 0x38, 0x60, 0xfe, 0x00 },
        { 0x0e, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0e, 0x00 },
        { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 },
        { 0x70, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x70, 0x00 },
        { 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    };

    static_assert(sizeof(FONT_DATA) == BYTES_PER_CHAR*NUM_CHARS, "sizeof FONT_DATA");
}

// Create a default font.
afl::base::Ref<gfx::Font>
gfx::createDefaultFont()
{
    afl::base::Ref<BitmapFont> font = *new BitmapFont();

    afl::charset::Unichar_t charCode = 32;
    for (size_t i = 0; i < NUM_CHARS; ++i) {
        font->addNewGlyph(charCode, new BitmapGlyph(8, BYTES_PER_CHAR, FONT_DATA[i]));
        ++charCode;
    }

    // Specials
    charCode = 0xE100;
    for (size_t i = 0; i < 16; ++i) {
        font->addNewGlyph(charCode, new BitmapGlyph(8, BYTES_PER_CHAR, FONT_DATA[31]));
        ++charCode;
    }

    return font;
}
