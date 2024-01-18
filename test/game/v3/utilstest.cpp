/**
  *  \file test/game/v3/utilstest.cpp
  *  \brief Test for game::v3::Utils
  */

#include "game/v3/utils.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"

using afl::base::Ref;
using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::io::ConstMemoryStream;
using afl::io::InternalDirectory;
using game::Player;
using game::PlayerList;

/** Test loadRaceNames(). */
AFL_TEST("game.v3.Utils:loadRaceNames", a)
{
    // Environment
    afl::string::NullTranslator tx;
    Ref<InternalDirectory> dir = InternalDirectory::create("spec");
    dir->addStream("race.nm", *new ConstMemoryStream(game::test::getDefaultRaceNames()));

    PlayerList pl;
    CodepageCharset cs(afl::charset::g_codepage437);

    // Test
    game::v3::loadRaceNames(pl, *dir, cs);

    // Verify
    a.checkNonNull("01. get", pl.get(1));
    a.checkEqual("02. name", pl.get(1)->getName(Player::ShortName, tx), "The Feds");

    a.checkNonNull("11. get", pl.get(11));
    a.checkEqual("12. name", pl.get(11)->getName(Player::ShortName, tx), "The Colonies");

    a.checkNonNull("21. get", pl.get(12));
    a.checkEqual("22. name", pl.get(12)->getName(Player::ShortName, tx), "Alien Marauders");

    a.checkNull("31. get", pl.get(13));
}

/** Test encryptTarget(). */
AFL_TEST("game.v3.Utils:encryptTarget", a)
{
    // Prepare
    static const uint8_t SPECIMEN[] = {
        0x1E, 0x01, 0x06, 0x00, 0x00, 0x00, 0x88, 0x09, 0x0B, 0x07, 0x34, 0x00, 0xFF, 0xFF, 0xCE, 0xEE,
        0xF1, 0xF9, 0xB6, 0xD7, 0xF8, 0xFC, 0xF1, 0xFA, 0xB0, 0xAF, 0xAE, 0xAD, 0xAC, 0xAB, 0xAA, 0xA9,
        0xA8, 0xA7,
    };
    game::v3::structures::ShipTarget t;
    afl::base::fromObject(t).copyFrom(SPECIMEN);

    // Test
    game::v3::encryptTarget(t);

    // Verify
    CodepageCharset cs(afl::charset::g_codepage437);
    a.checkEqual("01. decode", cs.decode(t.name), "Twin Block");

    // Test reversibility
    game::v3::encryptTarget(t);
    a.checkEqualContent("11. bytes", afl::base::ConstBytes_t(afl::base::fromObject(t)), afl::base::ConstBytes_t(SPECIMEN));
}
