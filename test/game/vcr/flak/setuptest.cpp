/**
  *  \file test/game/vcr/flak/setuptest.cpp
  *  \brief Test for game::vcr::flak::Setup
  */

#include "game/vcr/flak/setup.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    uint8_t FILE_CONTENT[] = {
        0xb8, 0x02, 0x00, 0x00, 0x23, 0x0a, 0xde, 0x09, 0xc9, 0x7a, 0x3d, 0x6d, 0x60, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x88, 0x02, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x64, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe0, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x65, 0x42, 0x00, 0x00, 0x29, 0x01, 0x00, 0x00, 0x09, 0x00, 0x04, 0x00, 0x02, 0x00, 0x64, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xe3, 0x55, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x04, 0x00, 0x06, 0x00, 0x02, 0x00, 0x64, 0x00, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
        0xa0, 0x92, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x42, 0x61, 0x72, 0x69, 0x75,
        0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x6e, 0x00,
        0x2b, 0x00, 0x09, 0x00, 0x51, 0x00, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x0c, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x64, 0x00, 0x01, 0x00, 0x83, 0x00, 0x00, 0x00,
        0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x47, 0x69, 0x62, 0x61, 0x72, 0x69, 0x61,
        0x6e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0xc9, 0x00,
        0x09, 0x00, 0x53, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x36, 0x00, 0x6f, 0x01, 0x64, 0x00, 0x10, 0x00, 0xf5, 0x01, 0x00, 0x00, 0xf4, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x52, 0x4b, 0x20, 0x4e, 0x69, 0x74, 0x72, 0x6f, 0x67, 0x65, 0x6e, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0b, 0x04, 0x36, 0x01, 0x09, 0x00,
        0x53, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x36, 0x00, 0xe2, 0x01, 0x64, 0x00, 0x10, 0x00, 0x7c, 0x02, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0xff, 0xff, 0x54, 0x68, 0x65, 0x74, 0x61, 0x20, 0x56, 0x49, 0x49, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0xba, 0x01, 0x09, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x07, 0x00, 0x0a, 0x00, 0x06, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0d, 0x00, 0x26, 0x00,
        0xe6, 0x00, 0x64, 0x00, 0x1a, 0x00, 0xca, 0x01, 0x00, 0x00, 0xf4, 0x01, 0x01, 0x00, 0x00, 0x00,
        0x52, 0x4b, 0x20, 0x56, 0x61, 0x6e, 0x64, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xb4, 0x02, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00,
        0x07, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x69, 0x00, 0x21, 0x03,
        0x64, 0x00, 0x20, 0x00, 0xe7, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x52, 0x4b,
        0x20, 0x53, 0x74, 0x72, 0x6f, 0x6e, 0x74, 0x69, 0x75, 0x6d, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x06, 0x08, 0xce, 0x03, 0x09, 0x00, 0x4f, 0x00, 0x01, 0x00, 0x07, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x5f, 0x00, 0x53, 0x03, 0x64, 0x00,
        0x20, 0x00, 0x19, 0x04, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x75, 0x72, 0x74,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x00, 0x00, 0x13, 0x03, 0x96, 0x01, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00,
        0x0d, 0x00, 0x59, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00,
        0xa2, 0x03, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x47, 0x72, 0x61, 0x75, 0x74, 0x76,
        0x6f, 0x72, 0x6e, 0x69, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00,
        0x13, 0x03, 0xd1, 0x02, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x09, 0x00, 0x0d, 0x00,
        0x64, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x02, 0x64, 0x00, 0x01, 0x00, 0xa2, 0x03,
        0x00, 0x00, 0xf4, 0x01, 0x00, 0x00, 0xff, 0xff, 0x06, 0x00, 0x32, 0x00, 0x07, 0x00, 0x2a, 0x00,
        0x06, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x1c, 0x00, 0x06, 0x00, 0x2f, 0x00, 0x07, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x01, 0x00, 0x32, 0x00, 0x02, 0x00, 0x3a, 0x00, 0x03, 0x00, 0x2a, 0x00,
        0x04, 0x00, 0x2a, 0x00, 0x05, 0x00, 0x2c, 0x00
    };
}

/** Test initialisation. */
AFL_TEST("game.vcr.flak.Setup:init", a)
{
    const game::vcr::flak::Setup        testee;
    a.checkEqual("01. getNumShips",     testee.getNumShips(), 0U);
    a.checkEqual("02. getNumFleets",    testee.getNumFleets(), 0U);
    a.checkEqual("03. getAttackList",   testee.getAttackList().size(), 0U);
    a.checkEqual("04. getAmbientFlags", testee.getAmbientFlags(), 0);
    a.checkEqual("05. getSeed",         testee.getSeed(), 0U);
    a.checkEqual("06. getTotalTime",    testee.getTotalTime(), 0);

    game::map::Point pt;
    a.checkEqual("11. getPosition", testee.getPosition().get(pt), false);
}

/** Test I/O.
    A: load and save a buffer.
    E: saved buffer must be the same as loaded buffer. */
AFL_TEST("game.vcr.flak.Setup:io", a)
{
    // Environment
    afl::string::NullTranslator tx;

    // Test
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs;
    testee.load("testIO", FILE_CONTENT, cs, tx);

    // Verify content
    game::map::Point pt;
    a.checkEqual("01. getAmbientFlags", testee.getAmbientFlags(), 0);
    a.checkEqual("02. getPosition",     testee.getPosition().get(pt), true);
    a.checkEqual("03. getX", pt.getX(), 2595);
    a.checkEqual("04. getY", pt.getY(), 2526);
    a.checkEqual("05. getTotalTime",    testee.getTotalTime(), 352);

    afl::base::GrowableBytes_t out;
    testee.save(out, cs);

    // Verify
    // Do not compare the first 16 bytes, which are size, x/y, seed, total_time.
    // total_time will not be set when we don't play the fight.
    const size_t OFFSET = 16;
    a.checkEqual("11. size", out.size(), sizeof(FILE_CONTENT));
    a.checkEqualContent("12. content",
                        afl::base::ConstBytes_t(out.subrange(OFFSET)),
                        afl::base::ConstBytes_t(afl::base::fromObject(FILE_CONTENT)).subrange(OFFSET));
}

/** Test copying. */
AFL_TEST("game.vcr.flak.Setup:copy", a)
{
    // Environment
    afl::string::NullTranslator tx;

    // Initialize by loading
    game::vcr::flak::Setup testee;
    afl::charset::Utf8Charset cs;
    testee.load("testCopy", FILE_CONTENT, cs, tx);

    // Make a copy
    game::vcr::flak::Setup t2(testee);

    // Verify the copy
    game::map::Point pt;
    a.checkEqual("01. getAmbientFlags", t2.getAmbientFlags(), 0);
    a.checkEqual("02. getPosition", t2.getPosition().get(pt), true);
    a.checkEqual("03. getX", pt.getX(), 2595);
    a.checkEqual("04. getY", pt.getY(), 2526);
    a.checkEqual("05. getTotalTime", t2.getTotalTime(), 352);
}
