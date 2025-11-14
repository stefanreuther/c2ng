/**
  *  \file test/game/map/ufotypetest.cpp
  *  \brief Test for game::map::UfoType
  */

#include "game/map/ufotype.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messageinformation.hpp"

namespace gp = game::parser;

using afl::base::Ref;
using game::map::Point;
using game::map::Ufo;
using game::config::HostConfiguration;

namespace {
    /*
     *  Data for merging tests, taken from Pleiades 13 @ PlanetsCentral, Turn 66, Crystal
     */
    const int TURN_NR = 66;

    /* Add Ufos. Emulates game::v3::Loader::loadUfos(). */
    void addUfos(afl::test::Assert a, game::map::UfoType& type)
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
        a.check("01. addUfo 53", pu);
        pu->setName("Wormhole #2");
        pu->setInfo1("15895 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x0b0b, 0x04e0));
        pu->setWarpFactor(0);
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
        a.check("11. addUfo 54", pu);
        pu->setName("Wormhole #3");
        pu->setInfo1("15895 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x09b2, 0x03f1));
        pu->setWarpFactor(0);
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
        a.check("21. addUfo 56", pu);
        pu->setName("Wormhole #5");
        pu->setInfo1("28142 KT/Bidir.");
        pu->setInfo2("mostly stable");
        pu->setPosition(Point(0x0b26, 0x043d));
        pu->setWarpFactor(0);
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
    void addHistory(afl::test::Assert a, game::map::UfoType& type)
    {
        Ufo* pu;
        pu = type.addUfo(51, 1, 2);
        a.check("31. addUfo 51", pu);
        pu->setPosition(Point(100,100));
        pu->setName("fifty-one");

        pu = type.addUfo(55, 1, 2);
        a.check("41. addUfo 55", pu);
        pu->setPosition(Point(200,200));
        pu->setName("fifty-five");
    }


    /*
     *  Movement Guessing Test
     */

    void doSingleMovementTest(afl::test::Assert a, int scan_x, int expected_result, int turns, const HostConfiguration& config)
    {
        // Environment
        afl::string::NullTranslator tx;

        // Name the sub test-case
        String_t name = afl::string::Format("scan=%d", scan_x);

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
            info.addValue(gp::mi_WarpFactor, 0);
            info.addValue(gp::mi_UfoShipRange, 150);
            info.addValue(gp::mi_UfoPlanetRange, 150);
            info.addValue(gp::mi_Radius, 5);
            info.addValue(gp::mi_UfoRealId, 9);
            info.addValue(gp::mi_UfoSpeedX, 7);
            info.addValue(gp::mi_UfoSpeedY, 7);
            p->addMessageInformation(info);
        }
        a(name).checkEqual("01. getMovementVector", p->getMovementVector().getX(), 7);
        a(name).checkEqual("02. getLastPosition",   p->getLastPosition().getX(), 2000);
        a(name).checkEqual("03. getLastTurn",       p->getLastTurn(), TURN_NR);

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
        a(name).check("11. isSeenThisTurn", p->isSeenThisTurn());

        Point pt;
        a(name).checkEqual("21. getPosition",       p->getPosition().get(pt), true);
        a(name).checkEqual("22. getX",              pt.getX(), scan_x);
        a(name).checkEqual("23. getMovementVector", p->getMovementVector().getX(), expected_result);
    }
}

/** Load Wormholes from Host-provided Ufos. */
AFL_TEST("game.map.UfoType:postprocess:wormhole-from-ufo", a)
{
    // Boilerplate
    Point pt;
    int r;
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addUfos(a, testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    a.check("01. getUfoByIndex", pu);
    a.checkEqual("02. getId", pu->getId(), 53);
    a.checkEqual("03. getPlainName", pu->getPlainName(), "Wormhole #2");
    a.checkEqual("04. getRealId", pu->getRealId(), 0);                   // not known in this case
    a.checkEqual("05. getInfo1", pu->getInfo1(), "15895 KT/Bidir.");
    a.checkEqual("06. getInfo2", pu->getInfo2(), "mostly stable");
    a.checkEqual("07. getPlanetRange", pu->getPlanetRange().orElse(-1), 251);
    a.checkEqual("08. getPosition", pu->getPosition().get(pt), true);
    a.checkEqual("09. position", pt, Point(2827, 1248));
    a.checkEqual("10. getRadius", pu->getRadius().get(r), true);
    a.checkEqual("11. radius", r, 6);
    a.checkNull("12. getOtherEnd", pu->getOtherEnd());  // not known for host ufos

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    a.check("21. getUfoByIndex", pu);
    a.checkEqual("22. getId", pu->getId(), 54);
    a.checkEqual("23. getPlainName", pu->getPlainName(), "Wormhole #3");
    a.checkEqual("24. getRealId", pu->getRealId(), 0);
    a.checkNull("25. getOtherEnd", pu->getOtherEnd());  // not known for host ufos

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    a.check("31. getUfoByIndex", pu);
    a.checkEqual("32. getId", pu->getId(), 56);
    a.checkEqual("33. getPlainName", pu->getPlainName(), "Wormhole #5");
    a.checkEqual("34. getRealId", pu->getRealId(), 0);
    a.checkNull("35. getOtherEnd", pu->getOtherEnd());  // not known for host ufos
}

/** Load Wormholes from util.dat wormhole records. */
AFL_TEST("game.map.UfoType:postprocess:wormholes-from-util", a)
{
    // Boilerplate
    Point pt;
    int r;
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    a.check("01. getUfoByIndex", pu);
    a.checkEqual("02. getId", pu->getId(), 53);
    a.checkEqual("03. getPlainName", pu->getPlainName(), "Wormhole #2");
    a.checkEqual("04. getRealId", pu->getRealId(), 2);
    a.checkEqual("05. getInfo1", pu->getInfo1(), "15895 kt/Bidir.");
    a.checkEqual("06. getInfo2", pu->getInfo2(), "mostly stable (<30%)");
    a.checkEqual("07. getPlanetRange", pu->getPlanetRange().orElse(-1), 251);
    a.checkEqual("08. getPosition", pu->getPosition().get(pt), true);
    a.checkEqual("09. position", pt, Point(2827, 1248));
    a.checkEqual("10. getRadius", pu->getRadius().get(r), true);
    a.checkEqual("11. radius", r, 6);
    a.checkNonNull("12. getOtherEnd", pu->getOtherEnd());
    a.checkEqual("13. getOtherEnd", pu->getOtherEnd()->getId(), 54);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    a.check("21. getUfoByIndex", pu);
    a.checkEqual("22. getId", pu->getId(), 54);
    a.checkEqual("23. getPlainName", pu->getPlainName(), "Wormhole #3");
    a.checkEqual("24. getRealId", pu->getRealId(), 3);
    a.checkNonNull("25. getOtherEnd", pu->getOtherEnd());
    a.checkEqual("26. getOtherEnd", pu->getOtherEnd()->getId(), 53);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    a.check("31. getUfoByIndex", pu);
    a.checkEqual("32. getId", pu->getId(), 56);
    a.checkEqual("33. getPlainName", pu->getPlainName(), "Wormhole #5");
    a.checkEqual("34. getRealId", pu->getRealId(), 5);
    a.checkNull("35. getOtherEnd", pu->getOtherEnd());
}

/** Load Wormholes from combined util.dat wormhole records and Host-provided Ufos. */
AFL_TEST("game.map.UfoType:postprocess:wormholes-from-both", a)
{
    // Boilerplate
    Point pt;
    int r;
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addUfos(a, testee);
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    a.check("01. getUfoByIndex", pu);
    a.checkEqual("02. getId", pu->getId(), 53);
    a.checkEqual("03. getPlainName", pu->getPlainName(), "Wormhole #2");
    a.checkEqual("04. getRealId", pu->getRealId(), 2);
    a.checkEqual("05. getInfo1", pu->getInfo1(), "15895 KT/Bidir.");       // from Host Ufo
    a.checkEqual("06. getInfo2", pu->getInfo2(), "mostly stable (<30%)");  // generated internally
    a.checkEqual("07. getPlanetRange", pu->getPlanetRange().orElse(-1), 251);
    a.checkEqual("08. getPosition", pu->getPosition().get(pt), true);
    a.checkEqual("09. position", pt, Point(2827, 1248));
    a.checkEqual("10. getRadius", pu->getRadius().get(r), true);
    a.checkEqual("11. radius", r, 6);
    a.checkNonNull("12. getOtherEnd", pu->getOtherEnd());
    a.checkEqual("13. getOtherEnd", pu->getOtherEnd()->getId(), 54);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    a.check("21. getUfoByIndex", pu);
    a.checkEqual("22. getId", pu->getId(), 54);
    a.checkEqual("23. getPlainName", pu->getPlainName(), "Wormhole #3");
    a.checkEqual("24. getRealId", pu->getRealId(), 3);
    a.checkNonNull("25. getOtherEnd", pu->getOtherEnd());
    a.checkEqual("26. getOtherEnd", pu->getOtherEnd()->getId(), 53);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    a.check("31. getUfoByIndex", pu);
    a.checkEqual("32. getId", pu->getId(), 56);
    a.checkEqual("33. getPlainName", pu->getPlainName(), "Wormhole #5");
    a.checkEqual("34. getRealId", pu->getRealId(), 5);
    a.checkNull("35. getOtherEnd", pu->getOtherEnd());
}

/** Load Wormholes from combined util.dat wormhole records and Host-provided Ufos,
    with history objects inbetween. This exercises how merging skips history objects. */
AFL_TEST("game.map.UfoType:postprocess:history", a)
{
    // Boilerplate
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    game::map::Configuration mapConfig;
    afl::sys::Log log;

    // Set up
    game::map::UfoType testee;
    addHistory(a, testee);
    addUfos(a, testee);
    addWormholes(testee);
    testee.postprocess(TURN_NR, mapConfig, config, tx, log);

    // Verify
    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(53));
    a.check("01. getUfoByIndex", pu);
    a.checkEqual("02. getId", pu->getId(), 53);
    a.checkEqual("03. getPlainName", pu->getPlainName(), "Wormhole #2");
    a.checkEqual("04. getRealId", pu->getRealId(), 2);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(54));
    a.check("11. getUfoByIndex", pu);
    a.checkEqual("12. getId", pu->getId(), 54);
    a.checkEqual("13. getPlainName", pu->getPlainName(), "Wormhole #3");
    a.checkEqual("14. getRealId", pu->getRealId(), 3);

    pu = testee.getUfoByIndex(testee.findUfoIndexById(56));
    a.check("21. getUfoByIndex", pu);
    a.checkEqual("22. getId", pu->getId(), 56);
    a.checkEqual("23. getPlainName", pu->getPlainName(), "Wormhole #5");
    a.checkEqual("24. getRealId", pu->getRealId(), 5);

    // History objects still there
    pu = testee.getUfoByIndex(testee.findUfoIndexById(51));
    a.check("31. getUfoByIndex", pu);
    a.checkEqual("32. getId", pu->getId(), 51);
    a.checkEqual("33. getPlainName", pu->getPlainName(), "fifty-one");

    pu = testee.getUfoByIndex(testee.findUfoIndexById(55));
    a.check("41. getUfoByIndex", pu);
    a.checkEqual("42. getId", pu->getId(), 55);
    a.checkEqual("43. getPlainName", pu->getPlainName(), "fifty-five");
}

/** Test movement guessing with Non-overlapping WrmDisplacement / WrmRandDisplacement. */
AFL_TEST("game.map.UfoType:movement-guessing:non-overlapping-config", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::WrmDisplacement].set(10);
    config[HostConfiguration::WrmRandDisplacement].set(2);

    // -----     -----     -----
    //   |---------|---------|
    doSingleMovementTest(a, 1988, -10, 1, config);
    doSingleMovementTest(a, 1989, -10, 1, config);
    doSingleMovementTest(a, 1990, -10, 1, config);
    doSingleMovementTest(a, 1991, -10, 1, config);
    doSingleMovementTest(a, 1992, -10, 1, config);
    doSingleMovementTest(a, 1993, -10, 1, config);
    doSingleMovementTest(a, 1994, -10, 1, config);
    doSingleMovementTest(a, 1995, -10, 1, config);
    doSingleMovementTest(a, 1996, -10, 1, config);
    doSingleMovementTest(a, 1997, -10, 1, config);
    doSingleMovementTest(a, 1998,   0, 1, config);
    doSingleMovementTest(a, 1999,   0, 1, config);
    doSingleMovementTest(a, 2000,   0, 1, config);
    doSingleMovementTest(a, 2001,   0, 1, config);
    doSingleMovementTest(a, 2002,   0, 1, config);
    doSingleMovementTest(a, 2003,  10, 1, config);
    doSingleMovementTest(a, 2004,  10, 1, config);
    doSingleMovementTest(a, 2005,  10, 1, config);
    doSingleMovementTest(a, 2006,  10, 1, config);
    doSingleMovementTest(a, 2007,  10, 1, config);
    doSingleMovementTest(a, 2008,  10, 1, config);
    doSingleMovementTest(a, 2009,  10, 1, config);
    doSingleMovementTest(a, 2010,  10, 1, config);
    doSingleMovementTest(a, 2011,  10, 1, config);
    doSingleMovementTest(a, 2012,  10, 1, config);
}

/* Test movement guessing with overlapping WrmDisplacement / WrmRandDisplacement. */
AFL_TEST("game.map.UfoType:movement-guessing:overlapping-config", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::WrmDisplacement].set(10);
    config[HostConfiguration::WrmRandDisplacement].set(7);

    //           ---------------
    // ---------------     ---------------
    //        |---------|---------|
    doSingleMovementTest(a, 1983, -10, 1, config);
    doSingleMovementTest(a, 1984, -10, 1, config);
    doSingleMovementTest(a, 1985, -10, 1, config);
    doSingleMovementTest(a, 1986, -10, 1, config);
    doSingleMovementTest(a, 1987, -10, 1, config);
    doSingleMovementTest(a, 1988, -10, 1, config);
    doSingleMovementTest(a, 1989, -10, 1, config);
    doSingleMovementTest(a, 1990, -10, 1, config);
    doSingleMovementTest(a, 1991, -10, 1, config);
    doSingleMovementTest(a, 1992, -10, 1, config);
    doSingleMovementTest(a, 1993,   7, 1, config); // ambiguous, could be -7 or -10+3
    doSingleMovementTest(a, 1994,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 1995,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 1996,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 1997,   7, 1, config); // ambiguous, could be -3 or -10+7
    doSingleMovementTest(a, 1998,   0, 1, config);
    doSingleMovementTest(a, 1999,   0, 1, config);
    doSingleMovementTest(a, 2000,   0, 1, config);
    doSingleMovementTest(a, 2001,   0, 1, config);
    doSingleMovementTest(a, 2002,   0, 1, config);
    doSingleMovementTest(a, 2003,   7, 1, config); // ambiguous, could be +7 or +10-3
    doSingleMovementTest(a, 2004,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 2005,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 2006,   7, 1, config); // ambiguous
    doSingleMovementTest(a, 2007,   7, 1, config); // ambiguous, could be +3 or +10-7
    doSingleMovementTest(a, 2008,  10, 1, config);
    doSingleMovementTest(a, 2009,  10, 1, config);
    doSingleMovementTest(a, 2010,  10, 1, config);
    doSingleMovementTest(a, 2011,  10, 1, config);
    doSingleMovementTest(a, 2012,  10, 1, config);
    doSingleMovementTest(a, 2013,  10, 1, config);
    doSingleMovementTest(a, 2014,  10, 1, config);
    doSingleMovementTest(a, 2015,  10, 1, config);
    doSingleMovementTest(a, 2016,  10, 1, config);
    doSingleMovementTest(a, 2017,  10, 1, config);
}

/* Test movement guessing with disabled displacement. */
AFL_TEST("game.map.UfoType:movement-guessing:no-displacement", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::WrmDisplacement].set(0);
    config[HostConfiguration::WrmRandDisplacement].set(7);

    // Result is always 0, Ufos do brownian motion only
    doSingleMovementTest(a, 1983,   0, 1, config);
    doSingleMovementTest(a, 1993,   0, 1, config);
    doSingleMovementTest(a, 2000,   0, 1, config);
    doSingleMovementTest(a, 2007,   0, 1, config);
    doSingleMovementTest(a, 2017,   0, 1, config);
}

/** Test iteration. */
AFL_TEST("game.map.UfoType:iteration", a)
{
    game::map::UfoType testee;
    Ufo* u10 = testee.addUfo(10, 1, 5);
    Ufo* u30 = testee.addUfo(30, 1, 7);
    Ufo* u20 = testee.addUfo(20, 1, 9);

    // Forward iteration
    {
        game::Id_t i = testee.getNextIndex(0);
        a.checkEqual("01. getObjectByIndex", testee.getObjectByIndex(i), u10);
        i = testee.getNextIndex(i);
        a.checkEqual("02. getObjectByIndex", testee.getObjectByIndex(i), u20);
        i = testee.getNextIndex(i);
        a.checkEqual("03. getObjectByIndex", testee.getObjectByIndex(i), u30);
        i = testee.getNextIndex(i);
        a.checkEqual("04. getNextIndex", i, 0);
    }

    // Backward iteration
    {
        game::Id_t i = testee.getPreviousIndex(0);
        a.checkEqual("11. getObjectByIndex", testee.getObjectByIndex(i), u30);
        i = testee.getPreviousIndex(i);
        a.checkEqual("12. getObjectByIndex", testee.getObjectByIndex(i), u20);
        i = testee.getPreviousIndex(i);
        a.checkEqual("13. getObjectByIndex", testee.getObjectByIndex(i), u10);
        i = testee.getPreviousIndex(i);
        a.checkEqual("14. getPreviousIndex", i, 0);
    }
}

AFL_TEST("game.map.UfoType:addMessageInformation", a)
{
    game::map::UfoType testee;

    {
        gp::MessageInformation info(gp::MessageInformation::Ufo, 20, TURN_NR);

        // Mandatory
        info.addValue(gp::mi_X, 1000);
        info.addValue(gp::mi_Y, 1200);
        info.addValue(gp::mi_Color, 5);
        info.addValue(gp::mi_Type, 15);
        info.addValue(gp::mi_Radius, 77);
        info.addValue(gp::ms_Name, "Weather balloon");

        // Optional
        info.addValue(gp::mi_Mass, 400);
        testee.addMessageInformation(info);
    }

    Ufo* pu = testee.getUfoByIndex(testee.findUfoIndexById(20));
    a.checkNonNull("01. getUfoByIndex", pu);
    a.checkEqual("02. getId", pu->getId(), 20);
    a.checkEqual("03. getPlainName", pu->getPlainName(), "Weather balloon");
    a.checkEqual("04. getColorCode", pu->getColorCode(), 5);
    a.checkEqual("05. getTypeCode", pu->getTypeCode().orElse(-1), 15);
    a.checkEqual("06. getRadius", pu->getRadius().orElse(-1), 77);
    a.checkEqual("07. getPosition", pu->getPosition().orElse(Point()), Point(1000, 1200));
}
