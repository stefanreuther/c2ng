/**
  *  \file test/game/vcr/flak/configurationtest.cpp
  *  \brief Test for game::vcr::flak::Configuration
  */

#include "game/vcr/flak/configuration.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.vcr.flak.Configuration:init", a)
{
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    a.checkEqual("01. RatingRandomBonus",    testee.RatingRandomBonus, 20);
    a.checkEqual("02. StartingDistanceShip", testee.StartingDistanceShip, 26000);
    a.checkEqual("03. SendUtilData",         testee.SendUtilData, true);
}

AFL_TEST("game.vcr.flak.Configuration:loadConfiguration", a)
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

    a.checkEqual("01. RatingRandomBonus", testee.RatingRandomBonus, 20);   // not changed, outside section
    a.checkEqual("02. CompensationLimit", testee.CompensationLimit, 600);
    a.checkEqual("03. MaximumFleetSize",  testee.MaximumFleetSize, 30);
    a.checkEqual("04. SendUtilData",      testee.SendUtilData, false);
    a.checkEqual("05. RatingTorpScale",   testee.RatingTorpScale, 1);      // not changed, outside section
}
