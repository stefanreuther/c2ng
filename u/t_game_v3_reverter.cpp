/**
  *  \file u/t_game_v3_reverter.cpp
  *  \brief Test for game::v3::Reverter
  */

#include "game/v3/reverter.hpp"

#include "t_game_v3.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

void
TestGameV3Reverter::testGetPreviousFriendlyCode()
{
    // Environment
    game::map::Universe univ;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Testee
    game::v3::Reverter testee(univ, session);

    // Add some objects
    {
        game::map::ShipData sd;
        sd.friendlyCode = String_t("s20");
        testee.addShipData(20, sd);

        sd.friendlyCode = String_t("s30");
        testee.addShipData(30, sd);

        game::map::PlanetData pd;
        pd.friendlyCode = String_t("p30");
        testee.addPlanetData(30, pd);

        pd.friendlyCode = String_t("p40");
        testee.addPlanetData(40, pd);
    }

    // Query
    TS_ASSERT_EQUALS(testee.getPreviousShipFriendlyCode(10).orElse("x"), "x");
    TS_ASSERT_EQUALS(testee.getPreviousShipFriendlyCode(20).orElse("x"), "s20");
    TS_ASSERT_EQUALS(testee.getPreviousShipFriendlyCode(30).orElse("x"), "s30");
    TS_ASSERT_EQUALS(testee.getPreviousShipFriendlyCode(40).orElse("x"), "x");

    TS_ASSERT_EQUALS(testee.getPreviousPlanetFriendlyCode(10).orElse("x"), "x");
    TS_ASSERT_EQUALS(testee.getPreviousPlanetFriendlyCode(20).orElse("x"), "x");
    TS_ASSERT_EQUALS(testee.getPreviousPlanetFriendlyCode(30).orElse("x"), "p30");
    TS_ASSERT_EQUALS(testee.getPreviousPlanetFriendlyCode(40).orElse("x"), "p40");

    // Totally out-of-range should not crash
    TS_ASSERT(!testee.getPreviousPlanetFriendlyCode(0).isValid());
    TS_ASSERT(!testee.getPreviousPlanetFriendlyCode(22222).isValid());
    TS_ASSERT(!testee.getPreviousPlanetFriendlyCode(-22222).isValid());
}
