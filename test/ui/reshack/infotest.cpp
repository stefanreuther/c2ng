/**
  *  \file test/ui/reshack/infotest.cpp
  *  \brief Test for ui::reshack::Info
  */

#include "ui/reshack/info.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/bitmapglyph.hpp"

using ui::reshack::Info;

namespace {
    const Info::Coverage* findCoverage(const std::vector<Info::Coverage>& vec, String_t name)
    {
        for (size_t i = 0; i < vec.size(); ++i) {
            if (vec[i].charsetName == name) {
                return &vec[i];
            }
        }
        return 0;
    }
}

/* Test getFontCoverage */
AFL_TEST("ui.reshack.Info:getFontCoverage", a)
{
    afl::string::NullTranslator tx;
    gfx::BitmapFont font;
    for (int i = 0; i < 256; ++i) {
        font.addNewGlyph(i, new gfx::BitmapGlyph(8, 8));
    }

    std::vector<Info::Coverage> cov = Info::getFontCoverage(font, tx);

    // ASCII
    const Info::Coverage* pa = findCoverage(cov, "ASCII");
    a.checkNonNull("ASCII must be present", pa);
    a.checkEqual("ASCII must be complete", pa->numMissingCharacters, 0U);

    // Latin-1 is missing U+03BC (GREEK SMALL LETTER MU) which we use instead of U+00B5 (MICRO SIGN)
    const Info::Coverage* pl = findCoverage(cov, "Latin-1");
    a.checkNonNull("Latin-1 must be present", pl);
    a.checkEqual("Latin-1 must be missing one", pl->numMissingCharacters, 1U);
    a.checkEqual("Latin-1 missing char", pl->firstMissingCharacter, 0x03BCU);

    // KOI8-R
    const Info::Coverage* pk = findCoverage(cov, "KOI8-R");
    a.checkNonNull("KOI8-R must be present", pk);
    a.checkEqual("KOI8-R must not be missing some", pk->numMissingCharacters, 122U);
    a.checkEqual("KOI8-R missing char", pk->firstMissingCharacter, 0x2500U);
}

/* Test getFontCoverage, missing ASCII chars */
AFL_TEST("ui.reshack.Info:getFontCoverage", a)
{
    afl::string::NullTranslator tx;
    gfx::BitmapFont font;
    for (int i = 0; i < 100; ++i) {
        font.addNewGlyph(i, new gfx::BitmapGlyph(8, 8));
    }

    std::vector<Info::Coverage> cov = Info::getFontCoverage(font, tx);

    const Info::Coverage* pa = findCoverage(cov, "ASCII");
    a.checkNonNull("ASCII must be present", pa);
    a.checkEqual("ASCII must be incomplete", pa->numMissingCharacters, 27U);
    a.checkEqual("ASCII missing char", pa->firstMissingCharacter, 100U);
}

/* Test getEncodingInfo */
AFL_TEST("ui.reshack.Info:getEncodingInfo", a)
{
    afl::string::NullTranslator tx;

    a.checkEqual("char 0041", Info::getEncodingInfo(0x41, tx),
                 "Unicode: U+0041 (65)\n"
                 "This is an ASCII character.\n"
                 "C encoding: '\\101', '\\x41'\n");

    a.checkEqual("char 0401", Info::getEncodingInfo(0x0401, tx),
                 "Unicode: U+0401 (1025)\n"
                 "UTF-8: '\\320\\201', '\\xD0\\x81'\n"
                 "CP1251: '\\250', '\\xA8'\n"
                 "CP866: '\\360', '\\xF0'\n"
                 "KOI8-R: '\\263', '\\xB3'\n");

    // PCC2 would report "not on any codepage" here
    a.checkEqual("char 6666", Info::getEncodingInfo(0x6666, tx),
                 "Unicode: U+6666 (26214)\n"
                 "UTF-8: '\\346\\231\\246', '\\xE6\\x99\\xA6'\n");
}
