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

/* loadConfiguration test, focus on syntax */
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

/* loadConfiguration test, focus on value coverage. */
AFL_TEST("game.vcr.flak.Configuration:loadConfiguration:full", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    static const char CONTENT[] =
        "% FLAK\n"
        "RatingMassScale            = 2\n"
        "RatingBeamScale            = 3\n"
        "RatingTorpScale            = 4\n"
        "RatingBayScale             = 5\n"
        "RatingPEBonus              = 11\n"
        "RatingFullAttackBonus      = 12\n"
        "RatingRandomBonus          = 13\n"
        "StartingDistanceShip       = 10000\n"
        "StartingDistancePlanet     = 5000\n"
        "StartingDistancePerPlayer  = 2000\n"
        "StartingDistancePerFleet   = 1000\n"
        "CompensationShipScale      = 20\n"
        "CompensationBeamScale      = 21\n"
        "CompensationTorpScale      = 22\n"
        "CompensationFighterScale   = 23\n"
        "CompensationLimit          = 666\n"
        "CompensationMass100KTScale = 777\n"
        "CompensationAdjust         = 999\n"
        "CyborgDebrisRate           = 88\n"
        "SendUtildata               = yes\n"
        "MaximumFleetSize           = 44\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(CONTENT));
    loadConfiguration(testee, ms, false, log, tx);

    a.checkEqual("RatingMassScale",            testee.RatingMassScale,              2);
    a.checkEqual("RatingBeamScale",            testee.RatingBeamScale,              3);
    a.checkEqual("RatingTorpScale",            testee.RatingTorpScale,              4);
    a.checkEqual("RatingBayScale",             testee.RatingBayScale,               5);
    a.checkEqual("RatingPEBonus",              testee.RatingPEBonus,               11);
    a.checkEqual("RatingFullAttackBonus",      testee.RatingFullAttackBonus,       12);
    a.checkEqual("RatingRandomBonus",          testee.RatingRandomBonus,           13);
    a.checkEqual("StartingDistanceShip",       testee.StartingDistanceShip,     10000);
    a.checkEqual("StartingDistancePlanet",     testee.StartingDistancePlanet,    5000);
    a.checkEqual("StartingDistancePerPlayer",  testee.StartingDistancePerPlayer, 2000);
    a.checkEqual("StartingDistancePerFleet",   testee.StartingDistancePerFleet,  1000);
    a.checkEqual("CompensationShipScale",      testee.CompensationShipScale,       20);
    a.checkEqual("CompensationBeamScale",      testee.CompensationBeamScale,       21);
    a.checkEqual("CompensationTorpScale",      testee.CompensationTorpScale,       22);
    a.checkEqual("CompensationFighterScale",   testee.CompensationFighterScale,    23);
    a.checkEqual("CompensationLimit",          testee.CompensationLimit,          666);
    a.checkEqual("CompensationMass100KTScale", testee.CompensationMass100KTScale, 777);
    a.checkEqual("CompensationAdjust",         testee.CompensationAdjust,         999);
    a.checkEqual("CyborgDebrisRate",           testee.CyborgDebrisRate,            88);
    a.checkEqual("SendUtildata",               testee.SendUtilData,              true);
    a.checkEqual("MaximumFleetSize",           testee.MaximumFleetSize,            44);
}

/* loadConfiguration test, error: bad number. */
AFL_TEST("game.vcr.flak.Configuration:loadConfiguration:error:number", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    static const char CONTENT[] =
        "% FLAK\n"
        "RatingMassScale = what?\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(CONTENT));
    AFL_CHECK_SUCCEEDS(a, loadConfiguration(testee, ms, false, log, tx));
}

/* loadConfiguration test, error: bad bool. */
AFL_TEST("game.vcr.flak.Configuration:loadConfiguration:error:bool", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    game::vcr::flak::Configuration testee;
    initConfiguration(testee);

    static const char CONTENT[] =
        "% FLAK\n"
        "SendUtilData = maybe?\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(CONTENT));
    AFL_CHECK_SUCCEEDS(a, loadConfiguration(testee, ms, false, log, tx));
}
