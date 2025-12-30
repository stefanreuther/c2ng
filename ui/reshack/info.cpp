/**
  *  \file ui/reshack/info.cpp
  *  \brief Class ui::reshack::Info
  */

#include "ui/reshack/info.hpp"

#include "afl/base/countof.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/format.hpp"
#include "util/charsetfactory.hpp"

using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::string::Format;
using util::CharsetFactory;

namespace {
    const uint16_t PCC_CHARS[] = {
        0x00AE, 1,                  // REGISTERED SIGN
        0x00B1, 1,                  // PLUS MINUS SIGN
        0x00B7, 1,                  // MIDDLE DOT
        0x00D7, 1,                  // MULTIPLICATION SIGN
        0x2013, 1,                  // EN DASH
        0x2022, 2,                  // BULLET (round), TRIANGULAR BULLET
        0x2122, 1,                  // TRADEMARK SIGN
        0x2190, 6,                  // LEFTWARDS ARROW, UPWARDS ARROW, RIGHTWARDS ARROW, DOWNWARDS ARROW, LEFT RIGHT ARROW, UP DOWN ARROW
        0x219E, 4,                  // LEFTWARDS TWO HEADED ARROW, UPWARDS TWO HEADED ARROW, RIGHTWARDS TWO HEADED ARROW, DOWNWARDS TWO HEADED ARROW
        0x2245, 1,                  // APPROXIMATELY EQUAL TO (=~)
        0x2248, 1,                  // ALMOST EQUAL TO (~~)
        0x2259, 1,                  // ESTIMATES (corresponds to) (=^)
        0x2264, 2,                  // LESS-THAN OR EQUAL TO (-<), GREATER-THAN OR EQUAL TO (->)
        0x25A0, 1,                  // SQUARE BULLET
        0x25B6, 1,                  // BLACK RIGHT-POINTING TRIANGLE (play)
        0x25BA, 1,                  // BLACK RIGHT-POINTING POINTER (bullet)
        0x25C0, 1,                  // BLACK LEFT-POINTING TRIANGLE (rew)
        0x2713, 2,                  // CHECK MARK
        0x2717, 1,                  // BALLOT CROSS
        0xE100, 16,                 // [PCC2] REPLACEMENT UPPER LEFT 0 (we only require the upper-left glyphs)
        0xE140, 5,                  // [PCC2] TOP ARC CLOCKWISE ARROW, NESTED RECTANGLES ORNAMENT, ORNAMENT LEFT, ORNAMENT RIGHT, THIN VERTICAL BAR (goes with BLACK TRIANGLE to form rewind-to-beginning)
        0,
    };

    const uint16_t WGL4_CHARS[] = {
        0x0020, 0x5F,               // Basic latin
        0x00A0, 0x60,               // Latin-1 supplement
        0x0100, 0x80,               // Latin Extended A
        0x0192, 1,                  // Latin Extended B
        0x01FA, 6,                  // Latin Extended B
        0x02C6, 2,                  // Spacing Modifier Letters
        0x02C9, 1,
        0x02D8, 6,
        0x0384, 7,                  // Greek
        0x038C, 1,
        0x038E, 20,
        0x03A3, 44,
        0x0400, 0x60,               // Cyrillic
        0x0490, 2,
        0x1E80, 6,                  // Latin Extended Additional
        0x1EF2, 2,
        0x2013, 3,                  // General Punctuation
        0x2017, 8,
        0x2020, 3,
        0x2026, 1,
        0x2030, 1,
        0x2032, 2,
        0x2039, 2,
        0x203C, 1,
        0x203E, 1,
        0x2044, 1,
        0x207F, 1,                  // Superscripts and Subscripts
        0x20A3, 2,                  // Currency Symbols
        0x20A7, 1,
        0x20AC, 1,
        0x2105, 1,                  // Letterlike Symbols
        0x2113, 1,
        0x2116, 1,
        0x2122, 1,
        0x2126, 1,
        0x212E, 1,
        0x215B, 4,                  // Number Forms
        0x2190, 6,                  // Arrows
        0x21A8, 0,
        0x2202, 1,                  // Mathematical Operators
        0x2206, 1,
        0x220F, 1,
        0x2211, 2,
        0x2215, 1,
        0x2219, 2,
        0x221F, 2,
        0x2229, 1,
        0x222B, 1,
        0x2248, 1,
        0x2260, 2,
        0x2264, 2,
        0x2302, 1,                  // Miscellaneous Technical
        0x2310, 1,
        0x2320, 2,
        0x2500, 1,                  // Box drawing
        0x2502, 1,
        0x250C, 1,
        0x2510, 1,
        0x2514, 1,
        0x2518, 1,
        0x251C, 1,
        0x2524, 1,
        0x252C, 1,
        0x2534, 1,
        0x253C, 1,
        0x2550, 0x1D,
        0x2580, 1,                  // Block Elements
        0x2584, 1,
        0x2588, 1,
        0x258C, 1,
        0x2590, 4,
        0x25A0, 2,                  // Geometric Shapes
        0x25AA, 3,
        0x25B2, 1,
        0x25BA, 1,
        0x25C4, 1,
        0x25CA, 2,
        0x25CF, 1,
        0x25D8, 2,
        0x25E6, 1,
        0x263A, 3,                  // Miscellaneous Symbols
        0x2640, 1,
        0x2642, 1,
        0x2660, 1,
        0x2663, 1,
        0x2665, 2,
        0x266A, 2,
        // 0xF001, 2,               // Private Use Area
        0xFB01, 2,                  // Alphabetic Presentation Forms (fi, fl)
        0,
    };

    const uint16_t MES1_CHARS[] = {
        0x0020, 95,
        0x00A0, 116,
        0x0116, 22,
        0x012E, 32,
        0x0150, 47,
        0x02C7, 1,
        0x02D8, 4,
        0x02DD, 1,
        0x2015, 1,
        0x2018, 2,
        0x201C, 2,
        0x20AC, 1,
        0x2122, 1,
        0x2126, 1,
        0x215B, 4,
        0x2190, 4,
        0x266A, 1,
        0,
    };

    const uint16_t MES2_CHARS[] = {
        0x0020, 95,
        0x00A0, 224,
        0x018F, 1,
        0x0192, 1,
        0x01B7, 1,
        0x01DE, 18,
        0x01FA, 6,
        0x0218, 4,
        0x021E, 2,
        0x0259, 1,
        0x027C, 1,
        0x0292, 1,
        0x02BB, 3,
        0x02C6, 2,
        0x02C9, 1,
        0x02D8, 6,
        0x02EE, 1,
        0x0374, 2,
        0x037A, 1,
        0x037E, 1,
        0x0384, 7,
        0x038C, 1,
        0x038E, 20,
        0x03A3, 44,
        0x03D7, 1,
        0x03DA, 8,
        0x0400, 96,
        0x0490, 53,
        0x04C7, 2,
        0x04CB, 2,
        0x04D0, 28,
        0x04EE, 8,
        0x04F8, 2,
        0x1E02, 2,
        0x1E0A, 2,
        0x1E1E, 2,
        0x1E40, 2,
        0x1E56, 2,
        0x1E60, 2,
        0x1E6A, 2,
        0x1E80, 6,
        0x1E9B, 1,
        0x1EF2, 2,
        0x1F00, 22,
        0x1F18, 6,
        0x1F20, 38,
        0x1F48, 6,
        0x1F50, 8,
        0x1F59, 1,
        0x1F5B, 1,
        0x1F5D, 1,
        0x1F5F, 31,
        0x1F80, 53,
        0x1FB6, 15,
        0x1FC6, 14,
        0x1FD6, 6,
        0x1FDD, 19,
        0x1FF2, 3,
        0x1FF6, 9,
        0x2013, 3,
        0x2017, 8,
        0x2020, 3,
        0x2026, 1,
        0x2030, 1,
        0x2032, 2,
        0x2039, 2,
        0x203C, 1,
        0x203E, 1,
        0x2044, 1,
        0x204A, 1,
        0x207F, 1,
        0x2082, 1,
        0x20A3, 2,
        0x20A7, 1,
        0x20AC, 1,
        0x20AF, 1,
        0x2105, 1,
        0x2116, 1,
        0x2122, 1,
        0x2126, 1,
        0x215B, 4,
        0x2190, 6,
        0x21A8, 1,
        0x2200, 1,
        0x2202, 2,
        0x2206, 1,
        0x2208, 2,
        0x220F, 1,
        0x2211, 2,
        0x2219, 2,
        0x221E, 2,
        0x2227, 5,
        0x2248, 1,
        0x2259, 1,
        0x2260, 2,
        0x2264, 2,
        0x2282, 2,
        0x2295, 1,
        0x2297, 1,
        0x2302, 1,
        0x2310, 1,
        0x2320, 2,
        0x2329, 2,
        0x2500, 1,
        0x2502, 1,
        0x250C, 1,
        0x2510, 1,
        0x2514, 1,
        0x2518, 1,
        0x251C, 1,
        0x2524, 1,
        0x252C, 1,
        0x2534, 1,
        0x253C, 1,
        0x2550, 29,
        0x2580, 1,
        0x2584, 1,
        0x2588, 1,
        0x258C, 1,
        0x2590, 4,
        0x25A0, 1,
        0x25AC, 1,
        0x25B2, 1,
        0x25BA, 1,
        0x25BC, 1,
        0x25C4, 1,
        0x25CA, 2,
        0x25D8, 2,
        0x263A, 3,
        0x2640, 1,
        0x2642, 1,
        0x2660, 1,
        0x2663, 1,
        0x2665, 2,
        0x266A, 2,
        0xFB01, 2,
        0xFFFD, 1,
        0,
    };

    const uint16_t MES3B_CHARS[] = {
        0x0020, 95,
        0x00A0, 384,
        0x0222, 18,
        0x0250, 94,
        0x02B0, 63,
        0x0300, 79,
        0x0360, 3,
        0x0374, 2,
        0x037A, 1,
        0x037E, 1,
        0x0384, 7,
        0x038C, 1,
        0x038E, 20,
        0x03A3, 44,
        0x03D0, 8,
        0x03DA, 26,
        0x0400, 135,
        0x0488, 2,
        0x048C, 57,
        0x04C7, 2,
        0x04CB, 2,
        0x04D0, 38,
        0x04F8, 2,
        0x0531, 38,
        0x0559, 7,
        0x0561, 39,
        0x0589, 2,
        0x10D0, 39,
        0x10FB, 1,
        0x1E00, 156,
        0x1EA0, 90,
        0x1F00, 22,
        0x1F18, 6,
        0x1F20, 38,
        0x1F48, 6,
        0x1F50, 8,
        0x1F59, 1,
        0x1F5B, 1,
        0x1F5D, 1,
        0x1F5F, 31,
        0x1F80, 53,
        0x1FB6, 15,
        0x1FC6, 14,
        0x1FD6, 6,
        0x1FDD, 19,
        0x1FF2, 3,
        0x1FF6, 9,
        0x2000, 71,
        0x2048, 6,
        0x206A, 7,
        0x2074, 27,
        0x20A0, 16,
        0x20D0, 20,
        0x2100, 59,
        0x2153, 49,
        0x2190, 100,
        0x2200, 242,
        0x2300, 124,
        0x237D, 30,
        0x2440, 11,
        0x2500, 150,
        0x25A0, 88,
        0x2600, 20,
        0x2619, 89,
        0xFB00, 7,
        0xFB13, 5,
        0xFE20, 4,
        0xFFF9, 5,
        0,
    };

    const uint16_t*const EXTRA_CHARSETS[] = {
        PCC_CHARS,
        WGL4_CHARS,
        MES1_CHARS,
        MES2_CHARS,
        MES3B_CHARS
    };

    const char*const EXTRA_CHARSET_NAMES[] = {
        "PCC2 Extras",
        "WGL4",
        "MES-1",
        "MES-2",
        "MES-3a",
    };
}

std::vector<ui::reshack::Info::Coverage>
ui::reshack::Info::getFontCoverage(const gfx::BitmapFont& font, afl::string::Translator& tx)
{
    // doFontCoverageDialog(const GfxBitmapFont& font, uint32_t& result) (part)
    std::vector<Coverage> result;

    // First: ASCII
    Unichar_t firstMissing = 0;
    size_t numMissing = 0;
    for (Unichar_t i = 32; i < 127; ++i) {
        if (font.getGlyph(i) == 0) {
            ++numMissing;
            if (numMissing == 1) {
                firstMissing = i;
            }
        }
    }
    result.push_back(Coverage("ASCII", numMissing, firstMissing));

    // Now the character sets
    CharsetFactory f;
    for (CharsetFactory::Index_t x = 0; x < f.getNumCharsets(); ++x) {
        std::auto_ptr<Charset> cs(f.createCharsetByIndex(x));
        CodepageCharset* cpcs = dynamic_cast<CodepageCharset*>(cs.get());
        if (cpcs != 0) {
            numMissing = 0;
            firstMissing = 0;
            for (Unichar_t i = 128; i < 256; ++i) {
                uint16_t ch = cpcs->get().m_characters[i-128];
                if (ch >= 0xA0 && font.getGlyph(ch) == 0) {
                    ++numMissing;
                    if (numMissing == 1) {
                        firstMissing = ch;
                    }
                }
            }
            result.push_back(Coverage(f.getCharsetName(x, tx), numMissing, firstMissing));
        }
    }

    // Finally, our specials
    for (size_t x = 0; x < countof(EXTRA_CHARSETS); ++x) {
        numMissing = 0;
        firstMissing = 0;
        const uint16_t* p = EXTRA_CHARSETS[x];
        while (*p != 0) {
            uint16_t ch = *p++;
            uint16_t count = *p++;
            while (count != 0) {
                if (font.getGlyph(ch) == 0) {
                    ++numMissing;
                    if (numMissing == 1) {
                        firstMissing = ch;
                    }
                }
                ++ch, --count;
            }
        }
        result.push_back(Coverage(EXTRA_CHARSET_NAMES[x], numMissing, firstMissing));
    }

    return result;
}

String_t
ui::reshack::Info::getEncodingInfo(Unichar_t ch, afl::string::Translator& tx)
{
    String_t text;
    text += Format(tx("Unicode: U+%04X (%0$0d)\n"), ch);
    if (ch < 128) {
        // ASCII
        text += Format(tx("This is an ASCII character.\n"
                          "C encoding: '\\%03o', '\\x%0$02X'\n"), ch);
    } else {
        // Encodings
        // (c2ng has UTF8 als encoding in CharsetFactory,
        // so we can make this very general unlike PCC2's somewhat more specific version.
        // However, we will never hit the "not in any codepage" case anymore.)
        String_t tmp;
        afl::charset::Utf8(0).append(tmp, ch);

        CharsetFactory f;
        for (CharsetFactory::Index_t x = 0; x < f.getNumCharsets(); ++x) {
            std::auto_ptr<Charset> cs(f.createCharsetByIndex(x));
            String_t encoded = afl::string::fromBytes(cs->encode(afl::string::toMemory(tmp)));
            if (!encoded.empty()) {
                text += Format("%s: '", f.getCharsetName(x, tx));
                for (String_t::size_type i = 0; i < encoded.size(); ++i) {
                    text += Format("\\%03o", uint8_t(encoded[i]));
                }
                text += "', '";
                for (String_t::size_type i = 0; i < encoded.size(); ++i) {
                    text += Format("\\x%02X", uint8_t(encoded[i]));
                }
                text += "'\n";
            }
        }
    }
    return text;
}
