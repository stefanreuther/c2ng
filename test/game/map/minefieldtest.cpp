/**
  *  \file test/game/map/minefieldtest.cpp
  *  \brief Test for game::map::Minefield
  *
  *  Test cases have been obtained using c2hosttest/mine/01_decay.
  */

#include "game/map/minefield.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/interpreterinterface.hpp"

using game::config::HostConfiguration;
using game::map::Minefield;
using game::map::Point;

/** Test mine decay, THost version. */
AFL_TEST("game.map.Minefield:getUnitsAfterDecay:Host", a)
{
    // Environment
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3, 22, 46));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    Minefield testee(7, Point(1000, 1000), 1, false, 200);

    // Test cases
    a.checkEqual("01", testee.getUnitsAfterDecay(  5, host, config),  4);
    a.checkEqual("02", testee.getUnitsAfterDecay( 10, host, config),  9);
    a.checkEqual("03", testee.getUnitsAfterDecay( 15, host, config), 13);
    a.checkEqual("04", testee.getUnitsAfterDecay( 20, host, config), 18);
    a.checkEqual("05", testee.getUnitsAfterDecay( 25, host, config), 23);
    a.checkEqual("06", testee.getUnitsAfterDecay( 30, host, config), 27);
    a.checkEqual("07", testee.getUnitsAfterDecay( 35, host, config), 32);
    a.checkEqual("08", testee.getUnitsAfterDecay( 40, host, config), 37);
    a.checkEqual("09", testee.getUnitsAfterDecay( 45, host, config), 42);
    a.checkEqual("10", testee.getUnitsAfterDecay( 50, host, config), 47);
    a.checkEqual("11", testee.getUnitsAfterDecay( 55, host, config), 51);
    a.checkEqual("12", testee.getUnitsAfterDecay( 60, host, config), 56);
    a.checkEqual("13", testee.getUnitsAfterDecay( 65, host, config), 61);
    a.checkEqual("14", testee.getUnitsAfterDecay( 70, host, config), 65);
    a.checkEqual("15", testee.getUnitsAfterDecay( 75, host, config), 70);
    a.checkEqual("16", testee.getUnitsAfterDecay( 80, host, config), 75);
    a.checkEqual("17", testee.getUnitsAfterDecay( 85, host, config), 80);
    a.checkEqual("18", testee.getUnitsAfterDecay( 90, host, config), 85);
    a.checkEqual("19", testee.getUnitsAfterDecay( 95, host, config), 89);
    a.checkEqual("20", testee.getUnitsAfterDecay(100, host, config), 94);
}

/** Test mine decay, PHost version. */
AFL_TEST("game.map.Minefield:getUnitsAfterDecay:PHost", a)
{
    // Environment
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4, 0, 0));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    Minefield testee(7, Point(1000, 1000), 1, false, 200);

    // Test cases
    a.checkEqual("01", testee.getUnitsAfterDecay(  5, host, config),  4);
    a.checkEqual("02", testee.getUnitsAfterDecay( 10, host, config),  9);
    a.checkEqual("03", testee.getUnitsAfterDecay( 15, host, config), 14);
    a.checkEqual("04", testee.getUnitsAfterDecay( 20, host, config), 19);
    a.checkEqual("05", testee.getUnitsAfterDecay( 25, host, config), 23);
    a.checkEqual("06", testee.getUnitsAfterDecay( 30, host, config), 28);
    a.checkEqual("07", testee.getUnitsAfterDecay( 35, host, config), 33);
    a.checkEqual("08", testee.getUnitsAfterDecay( 40, host, config), 38);
    a.checkEqual("09", testee.getUnitsAfterDecay( 45, host, config), 42);
    a.checkEqual("10", testee.getUnitsAfterDecay( 50, host, config), 47);
    a.checkEqual("11", testee.getUnitsAfterDecay( 55, host, config), 52);
    a.checkEqual("12", testee.getUnitsAfterDecay( 60, host, config), 57);
    a.checkEqual("13", testee.getUnitsAfterDecay( 65, host, config), 61);
    a.checkEqual("14", testee.getUnitsAfterDecay( 70, host, config), 66);
    a.checkEqual("15", testee.getUnitsAfterDecay( 75, host, config), 71);
    a.checkEqual("16", testee.getUnitsAfterDecay( 80, host, config), 76);
    a.checkEqual("17", testee.getUnitsAfterDecay( 85, host, config), 80);
    a.checkEqual("18", testee.getUnitsAfterDecay( 90, host, config), 85);
    a.checkEqual("19", testee.getUnitsAfterDecay( 95, host, config), 90);
    a.checkEqual("20", testee.getUnitsAfterDecay(100, host, config), 95);
}

/** Test initialisation, setter, getter. */
AFL_TEST("game.map.Minefield:init", a)
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;

    // Set up
    Minefield t(77, Point(2000, 3000), 4, true, 1024);

    // Verify getters
    a.checkEqual("01. getName", t.getName(game::PlainName, tx, iface), "Web Mine Field #77");
    a.checkEqual("02. getName", t.getName(game::DetailedName, tx, iface), "Web Mine Field #77");

    a.checkEqual("11. getId", t.getId(), 77);

    int n;
    a.checkEqual("21. getOwner", t.getOwner().get(n), true);
    a.checkEqual("22. owner", n, 4);

    Point pt;
    a.checkEqual("31. getPosition", t.getPosition().get(pt), true);
    a.checkEqual("32. position", pt, Point(2000, 3000));

    a.checkEqual("41. getRadius", t.getRadius().get(n), true);
    a.checkEqual("42. radius", n, 32);

    int32_t sq;
    a.checkEqual("51. getRadiusSquared", t.getRadiusSquared().get(sq), true);
    a.checkEqual("52. radius", sq, 1024);

    a.checkEqual("61. isValid", t.isValid(), true);

    a.checkEqual("71. isWeb", t.isWeb(), true);

    a.checkEqual("81. getReason", t.getReason(), Minefield::MinefieldScanned);

    a.checkEqual("91. getUnits", t.getUnits(), 1024);

    a.checkEqual("101. getTurnLastSeen", t.getTurnLastSeen(), 0);
    a.checkEqual("102. getUnits", t.getUnitsLastSeen(), 1024);

    // Modify units
    t.setUnits(2000);
    a.checkEqual("111. getUnitsLastSeen", t.getUnitsLastSeen(), 2000);
    a.checkEqual("112. getRadiusSquared", t.getRadiusSquared().get(sq), true);
    a.checkEqual("113. radius", sq, 2000);
    a.checkEqual("114. isValid", t.isValid(), true);

    // Copy
    Minefield u(t);
    a.checkEqual("121. getUnitsLastSeen", u.getUnitsLastSeen(), 2000);
    a.checkEqual("122. getRadiusSquared", u.getRadiusSquared().get(sq), true);
    a.checkEqual("123. radius", sq, 2000);
    a.checkEqual("124. isValid", u.isValid(), true);

    // Erase
    t.erase(0);
    a.checkEqual("131. isValid", t.isValid(), false);
}

/** Test initialisation, empty object. */
AFL_TEST("game.map.Minefield:init-empty", a)
{
    Minefield t(66);
    int n;
    a.checkEqual("01. getOwner", t.getOwner().get(n), false);
    a.checkEqual("02. isValid", t.isValid(), false);
}

/*
 *  addReport
 */

// Unit scan first, then radius scan within range -> unit scan kept
AFL_TEST("game.map.Minefield:addReport:unit-then-radius", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    game::config::HostConfiguration config;
    const int TURN = 5;

    Minefield m(10);
    m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 10005, TURN, Minefield::MinefieldScanned);
    m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::RadiusKnown, 100,  TURN, Minefield::MinefieldScanned);
    m.internalCheck(TURN, host, config);

    a.checkEqual("getUnits", m.getUnits(), 10005);
}

// Unit scan first, then radius scan outside range -> radius updated
AFL_TEST("game.map.Minefield:addReport:unit-then-outside-radius", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    game::config::HostConfiguration config;
    const int TURN = 5;

    Minefield m(10);
    m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 10005, TURN, Minefield::MinefieldScanned);
    m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::RadiusKnown, 80,  TURN, Minefield::MinefieldScanned);
    m.internalCheck(TURN, host, config);

    a.checkEqual("getUnits", m.getUnits(), 6400);
}

// Web field first, then neutral scan -> type kept
AFL_TEST("game.map.Minefield:addReport:web-then-neutral", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    game::config::HostConfiguration config;
    const int TURN = 5;

    Minefield m(10);
    m.addReport(Point(1000, 1000), 3, Minefield::IsWeb,       Minefield::UnitsKnown, 5000, TURN, Minefield::MinefieldScanned);
    m.addReport(Point(1000, 1000), 3, Minefield::UnknownType, Minefield::UnitsKnown, 4000, TURN, Minefield::MinefieldScanned);
    m.internalCheck(TURN, host, config);

    a.checkEqual("getUnits", m.getUnits(), 4000);
    a.checkEqual("isWeb", m.isWeb(), true);
}

// Web field first, then neutral scan, bot different position -> type reset
AFL_TEST("game.map.Minefield:addReport:web-then-neutral-elsewhere", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    game::config::HostConfiguration config;
    const int TURN = 5;

    Minefield m(10);
    m.addReport(Point(1000, 1000), 3, Minefield::IsWeb,       Minefield::UnitsKnown, 5000, TURN, Minefield::MinefieldScanned);
    m.addReport(Point(2000, 2000), 3, Minefield::UnknownType, Minefield::UnitsKnown, 4000, TURN, Minefield::MinefieldScanned);
    m.internalCheck(TURN, host, config);

    a.checkEqual("getUnits", m.getUnits(), 4000);
    a.checkEqual("isWeb", m.isWeb(), false);

    Point pt;
    a.checkEqual("getPosition", m.getPosition().get(pt), true);
    a.checkEqual("position", pt, Point(2000, 2000));
}

/** Test getPassRate(). */
AFL_TEST("game.map.Minefield:getPassRate", a)
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    HostConfiguration config;
    config[HostConfiguration::MineHitOdds].set(1);
    config[HostConfiguration::WebMineHitOdds].set(5);
    config[HostConfiguration::MineHitOddsWhenCloakedX10].set(5);

    // Normal: 0.99^10 = 0.9044
    // Cloaked: 0.995^10 = 0.9511
    {
        Minefield t(77, Point(2000, 3000), 4, false, 1024);
        a.checkNear("01. getPassRate", t.getPassRate(10, false, 3, config), 0.9044, 0.001);
        a.checkNear("02. getPassRate", t.getPassRate(10, true,  3, config), 0.9511, 0.001);
    }

    // Web: 0.95^10 = 0.5987
    {
        Minefield t(77, Point(2000, 3000), 4, true, 1024);
        a.checkNear("11. getPassRate", t.getPassRate(10, false, 3, config), 0.5987, 0.001);
        a.checkNear("12. getPassRate", t.getPassRate(10, true,  3, config), 0.5987, 0.001);
    }
}
