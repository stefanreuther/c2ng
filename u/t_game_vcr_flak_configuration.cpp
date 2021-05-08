/**
  *  \file u/t_game_vcr_flak_configuration.cpp
  *  \brief Test for game::vcr::flak::Configuration
  */

#include "game/vcr/flak/configuration.hpp"

#include "t_game_vcr_flak.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestGameVcrFlakConfiguration::testInit()
{
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    TS_ASSERT_EQUALS(testee.RatingRandomBonus, 20);
    TS_ASSERT_EQUALS(testee.StartingDistanceShip, 26000);
    TS_ASSERT_EQUALS(testee.SendUtilData, true);
}

void
TestGameVcrFlakConfiguration::testLoad()
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    static const char CONTENT[] =
        "RatingRandomBonus = 50\n"
        "%flak\n"
        "compensationLimit = 600\n"
        "whatever = 20\n"
        "MaximumFleetSize = 30\n"
        "SendUtilData = No\n"
        "%phost\n"
        "RatingTorpScale = 99\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(CONTENT));
    loadConfiguration(testee, ms, false, log, tx);

    TS_ASSERT_EQUALS(testee.RatingRandomBonus, 20);   // not changed, outside section
    TS_ASSERT_EQUALS(testee.CompensationLimit, 600);
    TS_ASSERT_EQUALS(testee.MaximumFleetSize, 30);
    TS_ASSERT_EQUALS(testee.SendUtilData, false);
    TS_ASSERT_EQUALS(testee.RatingTorpScale, 1);      // not changed, outside section
}

