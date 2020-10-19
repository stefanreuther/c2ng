/**
  *  \file u/t_game_map_ufotype.cpp
  *  \brief Test for game::map::UfoType
  */

#include "game/map/ufotype.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messageinformation.hpp"

namespace gp = game::parser;

using game::map::Point;
using game::map::Ufo;
using game::config::HostConfiguration;

namespace {
    /*
     *  Data for merging tests, taken from Pleiades 13 @ PlanetsCentral, Turn 66, Crystal
     */
    const int TURN_NR = 66;

    /* Add Ufos. Emulates game::v3::Loader::loadUfos(). */
    void addUfos(game::map::UfoType& type)
    {
        Ufo* pu;
        // Ufo 53:
        // 000a060:                               0200 576f  ..............Wo
        // 000a070: 726d 686f 6c65 2023 3220 2020 2020 2020  rmhole #2
        // 000a080: 2020 3135 3839 3520 4b54 2f42 6964 6972    15895 KT/Bidir
        // 000a090: 2e20 2020 2020 6d6f 7374 6c79 2073 7461  .     mostly sta
        // 000a0a0: 626c 6520 2020 2020 2020 0b0b e004 0000  ble       ......
        // 000a0b0: 0000 fb00 fb00 0600 0100
        pu = type.addUfo(53, 1, 2);
        TS_ASSERT(pu);
        pu->setName("Wormhole #2");
        pu->setInfo1("15895 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x0b0b, 0x04e0));
        pu->setSpeed(0);
        pu->setHeading(0);
        pu->setPlanetRange(0xfb);
        pu->setShipRange(0xfb);
        pu->setRadius(6);
        pu->setIsSeenThisTurn(true);

        // Ufo 54:
        // 000a0b0                           0200 576f 726d  ............Worm
        // 000a0c0: 686f 6c65 2023 3320 2020 2020 2020 2020  hole #3
        // 000a0d0: 3135 3839 3520 4b54 2f42 6964 6972 2e20  15895 KT/Bidir.
        // 000a0e0: 2020 2020 6d6f 7374 6c79 2073 7461 626c      mostly stabl
        // 000a0f0: 6520 2020 2020 2020 b209 f103 0000 0000  e       ........
        // 000a100: fb00 fb00 0600 0100
        pu = type.addUfo(54, 1, 2);
        TS_ASSERT(pu);
        pu->setName("Wormhole #3");
        pu->setInfo1("15895 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x09b2, 0x03f1));
        pu->setSpeed(0);
        pu->setHeading(0);
        pu->setPlanetRange(0xfb);
        pu->setShipRange(0xfb);
        pu->setRadius(6);
        pu->setIsSeenThisTurn(true);

        // Ufo 56:
        // 000a150:                0200 576f 726d 686f 6c65  ........Wormhole
        // 000a160: 2023 3520 2020 2020 2020 2020 3238 3134   #5         2814
        // 000a170: 3220 4b54 2f42 6964 6972 2e20 2020 2020  2 KT/Bidir.
        // 000a180: 6d6f 7374 6c79 2073 7461 626c 6520 2020  mostly stable
        // 000a190: 2020 2020 260b 3d04 0000 0000 3001 3001      &.=.....0.0.
        // 000a1a0: 0600 0100 0000 0000 0000 0000 0000 0000  ................
        pu = type.addUfo(56, 1, 2);
        TS_ASSERT(pu);
        pu->setName("Wormhole #5");
        pu->setInfo1("28142 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x0b26, 0x043d));
        pu->setSpeed(0);
        pu->setHeading(0);
        pu->setPlanetRange(0x130);
        pu->setShipRange(0x130);
        pu->setRadius(6);
        pu->setIsSeenThisTurn(true);
    }

    /* Add wormholes. Emulates game::v3::udata::Parser::handleRecord(14). */
    void addWormholes(game::map::UfoType& type)
    {
        // Type 14, Length 14 -- Wormhole Scan
        //   Position:       (2827, 1248)
        //   Mass:           15895
        //   Stability:      mostly stable (2)
        //   Id:             2
        //   Ufo Id:         53
        //   Bidirectional:  yes
        {
            gp::MessageInformation info(gp::MessageInformation::Wormhole, 2, TURN_NR);
            info.addValue(gp::mi_X, 2827);
            info.addValue(gp::mi_Y, 1248);
            info.addValue(gp::mi_Mass, 15895);
            info.addValue(gp::mi_WormholeStabilityCode, 2);
            info.addValue(gp::mi_UfoRealId, 53);
            info.addValue(gp::mi_WormholeBidirFlag, 1);
            type.addMessageInformation(info);
        }

        // Type 14, Length 14 -- Wormhole Scan
        //   Position:       (2482, 1009)
        //   Mass:           15895
        //   Stability:      mostly stable (2)
        //   Id:             3
        //   Ufo Id:         54
        //   Bidirectional:  yes
        {
            gp::MessageInformation info(gp::MessageInformation::Wormhole, 3, TURN_NR);
            info.addValue(gp::mi_X, 2482);
            info.addValue(gp::mi_Y, 1009);
            info.addValue(gp::mi_Mass, 15895);
            info.addValue(gp::mi_WormholeStabilityCode, 2);
            info.addValue(gp::mi_UfoRealId, 54);
            info.addValue(gp::mi_WormholeBidirFlag, 1);
            type.addMessageInformation(info);
        }

        // Type 14, Length 14 -- Wormhole Scan
        //   Position:       (2854, 1085)
        //   Mass:           28142
        //   Stability:      mostly stable (2)
        //   Id:             5
        //   Ufo Id:         56
        //   Bidirectional:  yes
        {
            gp::MessageInformation info(gp::MessageInformation::Wormhole, 5, TURN_NR);
            info.addValue(gp::mi_X, 2854);
            info.addValue(gp::mi_Y, 1085);
            info.addValue(gp::mi_Mass, 28142);
            info.addValue(gp::mi_WormholeStabilityCode, 2);
            info.addValue(gp::mi_UfoRealId, 56);
            info.addValue(gp::mi_WormholeBidirFlag, 1);
            type.addMessageInformation(info);
        }
    }

    /* Add history data.
       Simulates existence of a pre-existing database. */
    void addHistory(game::map::UfoType& type)
    {
        Ufo* pu;
        pu = type.addUfo(51, 1, 2);
        TS_ASSERT(pu);
        pu->setPosition(Point(100,100));
        pu->setName("fifty-one");

        pu = type.addUfo(55, 1, 2);
        TS_ASSERT(pu);
        pu->setPosition(Point(200,200));
        pu->setName("fifty-five");
    }


    /*
     *  Movement Guessing Test
     */

    void doSingleMovementTest(int scan_x, int expected_result, int turns, const char* name, const HostConfiguration& config)
    {
        // Environment
        afl::string::NullTranslator tx;

        // Create Ufo from history database
        game::map::UfoType type;
        const int UFO_ID = 60;

        Ufo* p = type.addUfo(UFO_ID, 1, 2);

        // Add history information (emulates game::db::Packer::addUfo)
        {
            gp::MessageInformation info(gp::MessageInformation::Ufo, UFO_ID, TURN_NR);
            info.addValue(gp::ms_Name, "Wormhole #9");
            info.addValue(gp::mi_X, 2000);
            info.addValue(gp::mi_Y, 2000);
            info.addValue(gp::mi_Speed, 0);
            info.addValue(gp::mi_UfoShipRange, 150);
            info.addValue(gp::mi_UfoPlanetRange, 150);
            info.addValue(gp::mi_Radius, 5);
            info.addValue(gp::mi_UfoRealId, 9);
            info.addValue(gp::mi_UfoSpeedX, 7);
            info.addValue(gp::mi_UfoSpeedY, 7);
            p->addMessageInformation(info);
        }
        TSM_ASSERT_EQUALS(name, p->getMovementVector().getX(), 7);
        TSM_ASSERT_EQUALS(name, p->getLastPosition().getX(), 2000);
        TSM_ASSERT_EQUALS(name, p->getLastTurn(), TURN_NR);

        // Add (modified) scan, similar to addWormholes
        {
            gp::MessageInformation info(gp::MessageInformation::Wormhole, 9, TURN_NR + turns);
            info.addValue(gp::mi_X, scan_x);
            info.addValue(gp::mi_Y, 2000);
            info.addValue(gp::mi_Mass, 5000);
            info.addValue(gp::mi_WormholeStabilityCode, 2);
            info.addValue(gp::mi_UfoRealId, UFO_ID);
            info.addValue(gp::mi_WormholeBidirFlag, 1);
            type.addMessageInformation(info);
        }

        // Perform postprocessing
        afl::sys::Log log;
        game::map::Configuration mapConfig;
        type.postprocess(TURN_NR + turns, mapConfig, config, tx, log);

        // Now check result
        TSM_ASSERT(name, p->isSeenThisTurn());

        Point pt;
        TSM_ASSERT_EQUALS(name, p->getPosition(pt), true);
        TSM_ASSERT_EQUALS(name, pt.getX(), scan_x);
        TSM_ASSERT_EQUALS(name, p->getMovementVector().getX(), expected_result);
    }
}

/** Load Wormholes from Host-provided Ufos. */
void
TestGameMapUfoType::testLoadUfo()
{
    // Boilerplate
    Point pt;
    int r;
    CxxTest::setAbortTestOnFail(true);
    afl::string::NullTranslator tx;
    HostConfiguration config;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addUfos(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 53);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #2");
    TS_ASSERT_EQUALS(pu->getRealId(), 0);                   // not known in this case
    TS_ASSERT_EQUALS(pu->getInfo1(), "15895 KT/Bidir.");
    TS_ASSERT_EQUALS(pu->getInfo2(), "mostly stable");
    TS_ASSERT_EQUALS(pu->getPlanetRange().orElse(-1), 251);
    TS_ASSERT_EQUALS(pu->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(2827, 1248));
    TS_ASSERT_EQUALS(pu->getRadius(r), true);
    TS_ASSERT_EQUALS(r, 6);
    TS_ASSERT(pu->getOtherEnd() == 0);  // not known for host ufos

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 54);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #3");
    TS_ASSERT_EQUALS(pu->getRealId(), 0);
    TS_ASSERT(pu->getOtherEnd() == 0);  // not known for host ufos

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 56);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #5");
    TS_ASSERT_EQUALS(pu->getRealId(), 0);
    TS_ASSERT(pu->getOtherEnd() == 0);  // not known for host ufos
}

/** Load Wormholes from util.dat wormhole records. */
void
TestGameMapUfoType::testLoadWormhole()
{
    // Boilerplate
    Point pt;
    int r;
    CxxTest::setAbortTestOnFail(true);
    afl::string::NullTranslator tx;
    HostConfiguration config;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 53);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #2");
    TS_ASSERT_EQUALS(pu->getRealId(), 2);
    TS_ASSERT_EQUALS(pu->getInfo1(), "15895 kt/Bidir.");
    TS_ASSERT_EQUALS(pu->getInfo2(), "mostly stable (<30%)");
    TS_ASSERT_EQUALS(pu->getPlanetRange().orElse(-1), 251);
    TS_ASSERT_EQUALS(pu->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(2827, 1248));
    TS_ASSERT_EQUALS(pu->getRadius(r), true);
    TS_ASSERT_EQUALS(r, 6);
    TS_ASSERT(pu->getOtherEnd() != 0);
    TS_ASSERT_EQUALS(pu->getOtherEnd()->getId(), 54);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 54);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #3");
    TS_ASSERT_EQUALS(pu->getRealId(), 3);
    TS_ASSERT(pu->getOtherEnd() != 0);
    TS_ASSERT_EQUALS(pu->getOtherEnd()->getId(), 53);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 56);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #5");
    TS_ASSERT_EQUALS(pu->getRealId(), 5);
    TS_ASSERT(pu->getOtherEnd() == 0);
}

/** Load Wormholes from combined util.dat wormhole records and Host-provided Ufos. */
void
TestGameMapUfoType::testLoadBoth()
{
    // Boilerplate
    Point pt;
    int r;
    CxxTest::setAbortTestOnFail(true);
    afl::string::NullTranslator tx;
    HostConfiguration config;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addUfos(testee);
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 53);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #2");
    TS_ASSERT_EQUALS(pu->getRealId(), 2);
    TS_ASSERT_EQUALS(pu->getInfo1(), "15895 KT/Bidir.");       // from Host Ufo
    TS_ASSERT_EQUALS(pu->getInfo2(), "mostly stable (<30%)");  // generated internally
    TS_ASSERT_EQUALS(pu->getPlanetRange().orElse(-1), 251);
    TS_ASSERT_EQUALS(pu->getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(2827, 1248));
    TS_ASSERT_EQUALS(pu->getRadius(r), true);
    TS_ASSERT_EQUALS(r, 6);
    TS_ASSERT(pu->getOtherEnd() != 0);
    TS_ASSERT_EQUALS(pu->getOtherEnd()->getId(), 54);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 54);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #3");
    TS_ASSERT_EQUALS(pu->getRealId(), 3);
    TS_ASSERT(pu->getOtherEnd() != 0);
    TS_ASSERT_EQUALS(pu->getOtherEnd()->getId(), 53);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 56);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #5");
    TS_ASSERT_EQUALS(pu->getRealId(), 5);
    TS_ASSERT(pu->getOtherEnd() == 0);
}

/** Load Wormholes from combined util.dat wormhole records and Host-provided Ufos,
    with history objects inbetween. This exercises how merging skips history objects. */
void
TestGameMapUfoType::testLoadHistory()
{
    // Boilerplate
    CxxTest::setAbortTestOnFail(true);
    afl::string::NullTranslator tx;
    HostConfiguration config;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addHistory(testee);
    addUfos(testee);
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 53);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #2");
    TS_ASSERT_EQUALS(pu->getRealId(), 2);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 54);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #3");
    TS_ASSERT_EQUALS(pu->getRealId(), 3);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 56);
    TS_ASSERT_EQUALS(pu->getPlainName(), "Wormhole #5");
    TS_ASSERT_EQUALS(pu->getRealId(), 5);

    // History objects still there
    pu = testee.getUfoByIndex(testee.findUfoIndexById(51));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 51);
    TS_ASSERT_EQUALS(pu->getPlainName(), "fifty-one");

    pu = testee.getUfoByIndex(testee.findUfoIndexById(55));
    TS_ASSERT(pu);
    TS_ASSERT_EQUALS(pu->getId(), 55);
    TS_ASSERT_EQUALS(pu->getPlainName(), "fifty-five");
}

#define XSTR(X) #X
#define STR(X) XSTR(X)
#define NAME __FILE__ ":" STR(__LINE__)

/** Test movement guessing with Non-overlapping WrmDisplacement / WrmRandDisplacement. */
void
TestGameMapUfoType::testMovementGuessing()
{
    HostConfiguration config;
    config[HostConfiguration::WrmDisplacement].set(10);
    config[HostConfiguration::WrmRandDisplacement].set(2);

    // -----     -----     -----
    //   |---------|---------|
    doSingleMovementTest(1988, -10, 1, NAME, config);
    doSingleMovementTest(1989, -10, 1, NAME, config);
    doSingleMovementTest(1990, -10, 1, NAME, config);
    doSingleMovementTest(1991, -10, 1, NAME, config);
    doSingleMovementTest(1992, -10, 1, NAME, config);
    doSingleMovementTest(1993, -10, 1, NAME, config);
    doSingleMovementTest(1994, -10, 1, NAME, config);
    doSingleMovementTest(1995, -10, 1, NAME, config);
    doSingleMovementTest(1996, -10, 1, NAME, config);
    doSingleMovementTest(1997, -10, 1, NAME, config);
    doSingleMovementTest(1998,   0, 1, NAME, config);
    doSingleMovementTest(1999,   0, 1, NAME, config);
    doSingleMovementTest(2000,   0, 1, NAME, config);
    doSingleMovementTest(2001,   0, 1, NAME, config);
    doSingleMovementTest(2002,   0, 1, NAME, config);
    doSingleMovementTest(2003,  10, 1, NAME, config);
    doSingleMovementTest(2004,  10, 1, NAME, config);
    doSingleMovementTest(2005,  10, 1, NAME, config);
    doSingleMovementTest(2006,  10, 1, NAME, config);
    doSingleMovementTest(2007,  10, 1, NAME, config);
    doSingleMovementTest(2008,  10, 1, NAME, config);
    doSingleMovementTest(2009,  10, 1, NAME, config);
    doSingleMovementTest(2010,  10, 1, NAME, config);
    doSingleMovementTest(2011,  10, 1, NAME, config);
    doSingleMovementTest(2012,  10, 1, NAME, config);
}

/* Test movement guessing with overlapping WrmDisplacement / WrmRandDisplacement. */
void
TestGameMapUfoType::testMovementGuessing2()
{
    HostConfiguration config;
    config[HostConfiguration::WrmDisplacement].set(10);
    config[HostConfiguration::WrmRandDisplacement].set(7);

    //           ---------------
    // ---------------     ---------------
    //        |---------|---------|
    doSingleMovementTest(1983, -10, 1, NAME, config);
    doSingleMovementTest(1984, -10, 1, NAME, config);
    doSingleMovementTest(1985, -10, 1, NAME, config);
    doSingleMovementTest(1986, -10, 1, NAME, config);
    doSingleMovementTest(1987, -10, 1, NAME, config);
    doSingleMovementTest(1988, -10, 1, NAME, config);
    doSingleMovementTest(1989, -10, 1, NAME, config);
    doSingleMovementTest(1990, -10, 1, NAME, config);
    doSingleMovementTest(1991, -10, 1, NAME, config);
    doSingleMovementTest(1992, -10, 1, NAME, config);
    doSingleMovementTest(1993,   7, 1, NAME, config); // ambiguous, could be -7 or -10+3
    doSingleMovementTest(1994,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(1995,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(1996,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(1997,   7, 1, NAME, config); // ambiguous, could be -3 or -10+7
    doSingleMovementTest(1998,   0, 1, NAME, config);
    doSingleMovementTest(1999,   0, 1, NAME, config);
    doSingleMovementTest(2000,   0, 1, NAME, config);
    doSingleMovementTest(2001,   0, 1, NAME, config);
    doSingleMovementTest(2002,   0, 1, NAME, config);
    doSingleMovementTest(2003,   7, 1, NAME, config); // ambiguous, could be +7 or +10-3
    doSingleMovementTest(2004,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(2005,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(2006,   7, 1, NAME, config); // ambiguous
    doSingleMovementTest(2007,   7, 1, NAME, config); // ambiguous, could be +3 or +10-7
    doSingleMovementTest(2008,  10, 1, NAME, config);
    doSingleMovementTest(2009,  10, 1, NAME, config);
    doSingleMovementTest(2010,  10, 1, NAME, config);
    doSingleMovementTest(2011,  10, 1, NAME, config);
    doSingleMovementTest(2012,  10, 1, NAME, config);
    doSingleMovementTest(2013,  10, 1, NAME, config);
    doSingleMovementTest(2014,  10, 1, NAME, config);
    doSingleMovementTest(2015,  10, 1, NAME, config);
    doSingleMovementTest(2016,  10, 1, NAME, config);
    doSingleMovementTest(2017,  10, 1, NAME, config);
}

/* Test movement guessing with disabled displacement. */
void
TestGameMapUfoType::testMovementGuessing3()
{
    HostConfiguration config;
    config[HostConfiguration::WrmDisplacement].set(0);
    config[HostConfiguration::WrmRandDisplacement].set(7);

    // Result is always 0, Ufos do brownian motion only
    doSingleMovementTest(1983,   0, 1, NAME, config);
    doSingleMovementTest(1993,   0, 1, NAME, config);
    doSingleMovementTest(2000,   0, 1, NAME, config);
    doSingleMovementTest(2007,   0, 1, NAME, config);
    doSingleMovementTest(2017,   0, 1, NAME, config);
}

/** Test iteration. */
void
TestGameMapUfoType::testIteration()
{
    game::map::UfoType testee;
    Ufo* u10 = testee.addUfo(10, 1, 5);
    Ufo* u30 = testee.addUfo(30, 1, 7);
    Ufo* u20 = testee.addUfo(20, 1, 9);

    // Forward iteration
    {
        game::Id_t i = testee.getNextIndex(0);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u10);
        i = testee.getNextIndex(i);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u20);
        i = testee.getNextIndex(i);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u30);
        i = testee.getNextIndex(i);
        TS_ASSERT_EQUALS(i, 0);
    }

    // Backward iteration
    {
        game::Id_t i = testee.getPreviousIndex(0);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u30);
        i = testee.getPreviousIndex(i);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u20);
        i = testee.getPreviousIndex(i);
        TS_ASSERT_EQUALS(testee.getObjectByIndex(i), u10);
        i = testee.getPreviousIndex(i);
        TS_ASSERT_EQUALS(i, 0);
    }
}
