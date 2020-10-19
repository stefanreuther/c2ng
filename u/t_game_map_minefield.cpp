/**
  *  \file u/t_game_map_minefield.cpp
  *  \brief Test for game::map::Minefield
  *
  *  Test cases have been obtained using c2hosttest/mine/01_decay.
  */

#include "game/map/minefield.hpp"

#include "t_game_map.hpp"
#include "game/test/interpreterinterface.hpp"
#include "afl/string/nulltranslator.hpp"

using game::config::HostConfiguration;
using game::map::Minefield;
using game::map::Point;

/** Test mine decay, THost version. */
void
TestGameMapMinefield::testUnitsAfterDecayHost()
{
    // Environment
    game::HostVersion host(game::HostVersion::Host, MKVERSION(3, 22, 46));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    Minefield testee(7, Point(1000, 1000), 1, false, 200);

    // Test cases
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(  5, host, config),  4);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 10, host, config),  9);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 15, host, config), 13);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 20, host, config), 18);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 25, host, config), 23);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 30, host, config), 27);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 35, host, config), 32);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 40, host, config), 37);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 45, host, config), 42);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 50, host, config), 47);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 55, host, config), 51);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 60, host, config), 56);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 65, host, config), 61);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 70, host, config), 65);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 75, host, config), 70);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 80, host, config), 75);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 85, host, config), 80);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 90, host, config), 85);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 95, host, config), 89);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(100, host, config), 94);
}

/** Test mine decay, PHost version. */
void
TestGameMapMinefield::testUnitsAfterDecayPHost()
{
    // Environment
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4, 0, 0));
    game::config::HostConfiguration config;
    config[config.MineDecayRate].set(5);

    // Minefield
    Minefield testee(7, Point(1000, 1000), 1, false, 200);

    // Test cases
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(  5, host, config),  4);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 10, host, config),  9);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 15, host, config), 14);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 20, host, config), 19);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 25, host, config), 23);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 30, host, config), 28);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 35, host, config), 33);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 40, host, config), 38);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 45, host, config), 42);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 50, host, config), 47);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 55, host, config), 52);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 60, host, config), 57);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 65, host, config), 61);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 70, host, config), 66);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 75, host, config), 71);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 80, host, config), 76);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 85, host, config), 80);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 90, host, config), 85);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay( 95, host, config), 90);
    TS_ASSERT_EQUALS(testee.getUnitsAfterDecay(100, host, config), 95);
}

/** Test initialisation, setter, getter. */
void
TestGameMapMinefield::testInit()
{
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;

    // Set up
    Minefield t(77, Point(2000, 3000), 4, true, 1024);

    // Verify getters
    TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Web Mine Field #77");
    TS_ASSERT_EQUALS(t.getName(game::DetailedName, tx, iface), "Web Mine Field #77");

    TS_ASSERT_EQUALS(t.getId(), 77);

    int n;
    TS_ASSERT_EQUALS(t.getOwner(n), true);
    TS_ASSERT_EQUALS(n, 4);

    Point pt;
    TS_ASSERT_EQUALS(t.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(2000, 3000));

    TS_ASSERT_EQUALS(t.getRadius(n), true);
    TS_ASSERT_EQUALS(n, 32);

    int32_t sq;
    TS_ASSERT_EQUALS(t.getRadiusSquared(sq), true);
    TS_ASSERT_EQUALS(sq, 1024);

    TS_ASSERT_EQUALS(t.isValid(), true);

    TS_ASSERT_EQUALS(t.isWeb(), true);

    TS_ASSERT_EQUALS(t.getReason(), Minefield::MinefieldScanned);

    TS_ASSERT_EQUALS(t.getUnits(), 1024);

    TS_ASSERT_EQUALS(t.getTurnLastSeen(), 0);
    TS_ASSERT_EQUALS(t.getUnitsLastSeen(), 1024);

    // Modify units
    t.setUnits(2000);
    TS_ASSERT_EQUALS(t.getUnitsLastSeen(), 2000);
    TS_ASSERT_EQUALS(t.getRadiusSquared(sq), true);
    TS_ASSERT_EQUALS(sq, 2000);
    TS_ASSERT_EQUALS(t.isValid(), true);

    // Copy
    Minefield u(t);
    TS_ASSERT_EQUALS(u.getUnitsLastSeen(), 2000);
    TS_ASSERT_EQUALS(u.getRadiusSquared(sq), true);
    TS_ASSERT_EQUALS(sq, 2000);
    TS_ASSERT_EQUALS(u.isValid(), true);

    // Erase
    t.erase();
    TS_ASSERT_EQUALS(t.isValid(), false);
}

/** Test initialisation, empty object. */
void
TestGameMapMinefield::testInitEmpty()
{
    Minefield t(66);
    int n;
    TS_ASSERT_EQUALS(t.getOwner(n), false);
    TS_ASSERT_EQUALS(t.isValid(), false);
}

/** Test addReport(). */
void
TestGameMapMinefield::testAddReport()
{
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));
    game::config::HostConfiguration config;
    const int TURN = 5;

    // Unit scan first, then radius scan within range -> unit scan kept
    {
        Minefield m(10);
        m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 10005, TURN, Minefield::MinefieldScanned);
        m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::RadiusKnown, 100,  TURN, Minefield::MinefieldScanned);
        m.internalCheck(TURN, host, config);

        TS_ASSERT_EQUALS(m.getUnits(), 10005);
    }

    // Unit scan first, then radius scan outside range -> radius updated
    {
        Minefield m(10);
        m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::UnitsKnown, 10005, TURN, Minefield::MinefieldScanned);
        m.addReport(Point(1000, 1000), 3, Minefield::IsMine, Minefield::RadiusKnown, 80,  TURN, Minefield::MinefieldScanned);
        m.internalCheck(TURN, host, config);

        TS_ASSERT_EQUALS(m.getUnits(), 6400);
    }

    // Web field first, then neutral scan -> type kept
    {
        Minefield m(10);
        m.addReport(Point(1000, 1000), 3, Minefield::IsWeb,       Minefield::UnitsKnown, 5000, TURN, Minefield::MinefieldScanned);
        m.addReport(Point(1000, 1000), 3, Minefield::UnknownType, Minefield::UnitsKnown, 4000, TURN, Minefield::MinefieldScanned);
        m.internalCheck(TURN, host, config);

        TS_ASSERT_EQUALS(m.getUnits(), 4000);
        TS_ASSERT_EQUALS(m.isWeb(), true);
    }

    // Web field first, then neutral scan, bot different position -> type reset
    {
        Minefield m(10);
        m.addReport(Point(1000, 1000), 3, Minefield::IsWeb,       Minefield::UnitsKnown, 5000, TURN, Minefield::MinefieldScanned);
        m.addReport(Point(2000, 2000), 3, Minefield::UnknownType, Minefield::UnitsKnown, 4000, TURN, Minefield::MinefieldScanned);
        m.internalCheck(TURN, host, config);

        TS_ASSERT_EQUALS(m.getUnits(), 4000);
        TS_ASSERT_EQUALS(m.isWeb(), false);

        Point pt;
        TS_ASSERT_EQUALS(m.getPosition(pt), true);
        TS_ASSERT_EQUALS(pt, Point(2000, 2000));
    }
}

/** Test getPassRate(). */
void
TestGameMapMinefield::testGetPassRate()
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
        TS_ASSERT_DELTA(t.getPassRate(10, false, 3, config), 0.9044, 0.001);
        TS_ASSERT_DELTA(t.getPassRate(10, true,  3, config), 0.9511, 0.001);
    }

    // Web: 0.95^10 = 0.5987
    {
        Minefield t(77, Point(2000, 3000), 4, true, 1024);
        TS_ASSERT_DELTA(t.getPassRate(10, false, 3, config), 0.5987, 0.001);
        TS_ASSERT_DELTA(t.getPassRate(10, true,  3, config), 0.5987, 0.001);
    }
}

