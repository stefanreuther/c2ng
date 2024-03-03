/**
  *  \file test/game/map/shippredictortest.cpp
  *  \brief Test for game::map::ShipPredictor
  */

#include "game/map/shippredictor.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace {
    using game::HostVersion;
    using game::map::Point;
    using game::map::Ship;
    using game::map::ShipPredictor;
    using game::Element;
    using game::config::HostConfiguration;
    using game::spec::BasicHullFunction;

    const int X = 1200;
    const int Y = 1300;

    struct TestHarness {
        game::map::Universe univ;
        game::map::Configuration mapConfig;
        game::UnitScoreDefinitionList shipScores;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion hostVersion;
        game::test::RegistrationKey key;

        TestHarness()
            : univ(), mapConfig(), shipScores(), shipList(), config(), hostVersion(), key(game::RegistrationKey::Unknown, 6)
            { }
    };

    Ship& addEmerald(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Emerald (from game "Schule", turn 61, ship #348)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 6;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(480);
        h.setMaxCargo(510);
        h.setMaxCrew(258);
        h.setNumEngines(2);
        h.setMass(218);        // we'll not add weapons; the plain hull only weighs 180 kt

        // HeavyNovaDrive 6:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(9, 72900);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    Ship& addCarrier(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Golem
         */
        const int HULL_ID = 79;
        const int ENGINE_ID = 6;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(2000);
        h.setMaxCargo(300);
        h.setMaxCrew(1958);
        h.setNumEngines(8);
        h.setMass(850);

        // HeavyNovaDrive 6:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(9, 72900);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(10);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    Ship& addJumper(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Hyperjumper (Heart of Gold from Pleiades 7, turn 38, ship #299)
         */
        const int HULL_ID = 51;
        const int ENGINE_ID = 1;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(95);
        h.setMaxCargo(20);
        h.setMaxCrew(25);
        h.setNumEngines(1);
        h.setMass(138);
        h.changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Hyperdrive),
                             game::PlayerSet_t::allUpTo(game::MAX_PLAYERS),
                             game::PlayerSet_t(),
                             true);

        // Impulse Drive:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(1, 100);
        e.setFuelFactor(2, 800);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(1);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 60);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    Ship& addMerlin(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Merlin (from game "qvs0", turn 110, ship #2)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 9;

        // Emerald:
        game::spec::Hull& h = *t.shipList.hulls().create(HULL_ID);
        h.setMaxFuel(450);
        h.setMaxCargo(2700);
        h.setMaxCrew(120);
        h.setNumEngines(10);
        h.setMass(928);        // we'll not add weapons; the plain hull only weighs 920 kt

        // Transwarp Drive:
        game::spec::Engine& e = *t.shipList.engines().create(ENGINE_ID);
        e.setFuelFactor(9, 8100);

        // Add a ship
        // - required properties
        Ship& s = *t.univ.ships().create(shipId);
        s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s.setOwner(1);
        s.setHull(HULL_ID);
        s.setEngineType(ENGINE_ID);
        s.setPosition(game::map::Point(X, Y));
        s.setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s.setBeamType(0);
        s.setNumBeams(0);
        s.setTorpedoType(0);
        s.setNumLaunchers(0);
        s.setNumBays(0);
        s.setCargo(Element::Neutronium, 100);
        s.setCargo(Element::Tritanium, 0);
        s.setCargo(Element::Duranium, 0);
        s.setCargo(Element::Molybdenum, 0);
        s.setCargo(Element::Supplies, 0);
        s.setCargo(Element::Money, 0);
        s.setCargo(Element::Colonists, 0);
        s.setAmmo(0);

        return s;
    }

    void finish(TestHarness& t)
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        t.univ.postprocess(game::PlayerSet_t::allUpTo(11),      // playingSet
                           game::PlayerSet_t::allUpTo(11),      // availablePlayers
                           game::map::Object::Editable,         // playability
                           t.mapConfig,                         // mapConfig
                           t.hostVersion,                       // host
                           t.config,                            // config
                           42,                                  // turn
                           t.shipList,                          // shipList
                           tx,                                  // translator
                           log);                                // log
    }

    /* Canned test case: ship moving X light years burns Y kt fuel
       (THost distance anomaly) */
    void testFuelUsage(afl::test::Assert a, int distance, int expected)
    {
        const int SHIP_ID = 348;

        TestHarness t;
        t.hostVersion = HostVersion(HostVersion::Host, MKVERSION(3,22,0));
        Ship& s = addEmerald(t, SHIP_ID);
        s.setWaypoint(Point(X + distance, Y));

        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        a.checkEqual("getMovementFuelUsed", testee.getMovementFuelUsed(), expected);
        a.checkEqual("getCloakFuelUsed",    testee.getCloakFuelUsed(), 0);
        a.checkEqual("getNumTurns",         testee.getNumTurns(), 1);
        a.checkEqual("isAtTurnLimit",       testee.isAtTurnLimit(), false);
    }

    /* Canned test case: ship having HAVE fuel needs NEED
       (PHost fuel consumption anomaly) */
    void testFuelUsagePHost(afl::test::Assert a, int have, int need, HostVersion version)
    {
        const int SHIP_ID = 2;

        TestHarness t;
        t.hostVersion = version;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setWaypoint(Point(X + 75, Y + 34));
        s.setCargo(Element::Neutronium, have);
        t.config[HostConfiguration::UseAccurateFuelModel].set(true);

        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        a.checkEqual("getMovementFuelUsed", testee.getMovementFuelUsed(), need);
        a.checkEqual("getCloakFuelUsed",    testee.getCloakFuelUsed(), 0);
        a.checkEqual("getNumTurns",         testee.getNumTurns(), 1);
        a.checkEqual("isAtTurnLimit",       testee.isAtTurnLimit(), false);
    }

    /* Canned test case: alchemy friendly codes */
    void testAlchemy(afl::test::Assert a, const char* friendlyCode, int suppliesBefore, int tritaniumAfter, int duraniumAfter, int molybdenumAfter, int suppliesAfter, HostVersion host, bool expectAlchemy, bool expectFriendlyCode)
    {
        const int SHIP_ID = 59;

        String_t label = afl::string::Format("%s fc=%s s=%d", host.toString(), friendlyCode, suppliesBefore);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1);
        s.setCargo(Element::Tritanium, 10);
        s.setCargo(Element::Duranium, 20);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Supplies, suppliesBefore);
        s.setFriendlyCode(String_t(friendlyCode));

        t.shipList.hulls().get(61)->changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::MerlinAlchemy),
                                                       game::PlayerSet_t::fromInteger(-1), game::PlayerSet_t(), true);

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
        testee.computeTurn();

        a(label).checkEqual("Tritanium",   testee.getCargo(Element::Tritanium),  tritaniumAfter);
        a(label).checkEqual("Duranium",    testee.getCargo(Element::Duranium),   duraniumAfter);
        a(label).checkEqual("Molybdenum",  testee.getCargo(Element::Molybdenum), molybdenumAfter);
        a(label).checkEqual("Supplies",    testee.getCargo(Element::Supplies),   suppliesAfter);
        a(label).checkEqual("UsedAlchemy", testee.getUsedProperties().contains(ShipPredictor::UsedAlchemy), expectAlchemy);
        a(label).checkEqual("UsedFCode",   testee.getUsedProperties().contains(ShipPredictor::UsedFCode), expectFriendlyCode);
    }

    /* Canned test case: refinery friendly codes */
    void testGenericRefinery(afl::test::Assert a, const char* friendlyCode, int suppliesBefore, int tritaniumAfter, int duraniumAfter, int molybdenumAfter, int suppliesAfter, int fuelAfter, HostVersion host, bool expectAlchemy, bool expectFriendlyCode, afl::base::Memory<const int> hullFuncs)
    {
        const int SHIP_ID = 59;

        String_t label = afl::string::Format("%s fc=%s s=%d", host.toString(), friendlyCode, suppliesBefore);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1);
        s.setCargo(Element::Tritanium, 10);
        s.setCargo(Element::Duranium, 20);
        s.setCargo(Element::Molybdenum, 30);
        s.setCargo(Element::Supplies, suppliesBefore);
        s.setFriendlyCode(String_t(friendlyCode));

        while (const int* p = hullFuncs.eat()) {
            t.shipList.hulls().get(61)->changeHullFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(*p), game::PlayerSet_t::fromInteger(-1), game::PlayerSet_t(), true);
        }

        finish(t);

        game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
        testee.computeTurn();

        a(label).checkEqual("Tritanium",   testee.getCargo(Element::Tritanium),  tritaniumAfter);
        a(label).checkEqual("Duranium",    testee.getCargo(Element::Duranium),   duraniumAfter);
        a(label).checkEqual("Molybdenum",  testee.getCargo(Element::Molybdenum), molybdenumAfter);
        a(label).checkEqual("Supplies",    testee.getCargo(Element::Supplies),   suppliesAfter);
        a(label).checkEqual("Neutronium",  testee.getCargo(Element::Neutronium), fuelAfter);
        a(label).checkEqual("UsedAlchemy", testee.getUsedProperties().contains(ShipPredictor::UsedAlchemy), expectAlchemy);
        a(label).checkEqual("UsedFCode",   testee.getUsedProperties().contains(ShipPredictor::UsedFCode), expectFriendlyCode);
    }

    void testMovement2(afl::test::Assert a, int waypointDX, int waypointDY, int warp, int movedDX, int movedDY, HostVersion host)
    {
        const int SHIP_ID = 77;

        String_t label = afl::string::Format("%s %d,%d", host.toString(), waypointDX, waypointDY);

        TestHarness t;
        t.hostVersion = host;
        Ship& s = addMerlin(t, SHIP_ID);
        s.setCargo(Element::Neutronium, 1000);
        s.setWarpFactor(warp);
        s.setWaypoint(Point(X + waypointDX, Y + waypointDY));
        finish(t);

        ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        a(label).checkEqual("getX", testee.getPosition().getX(), X + movedDX);
        a(label).checkEqual("getY", testee.getPosition().getY(), Y + movedDY);
    }
}

/** Test error cases. ShipPredictor must not crash or hang. */

// Non-existant ship
AFL_TEST("game.map.ShipPredictor:error:no-ship", a)
{
    TestHarness t;
    ShipPredictor p(t.univ, 99, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();
    p.computeTurn();
    a.checkEqual("01. getNumTurns", p.getNumTurns(), 0);

    // For coverage...
    a.checkEqual("11. getUsedProperties", &p.getUniverse(), &t.univ);
    a.checkEqual("12. getTowedShipName", p.getTowedShipName(), "");
}

// Ship exists but hull doesn't.
AFL_TEST_NOARG("game.map.ShipPredictor:error:no-hull")
{
    const int SHIP_ID = 32;
    TestHarness t;
    Ship& s = *t.univ.ships().create(SHIP_ID);
    s.addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
    s.setOwner(1);
    s.setHull(77);
    s.setEngineType(7);
    s.setPosition(game::map::Point(1000, 1000));
    s.setWaypoint(game::map::Point(1200, 1200));
    s.setWarpFactor(9);

    {
        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
        p.computeMovement();
    }
    {
        ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
        p.computeTurn();
    }
}

/** Test fuel usage computation for THost.
    This checks the distance computation anomaly: a ship moving 3.00 ly burns the same
    amount of fuel as one moving 2.00 ly. */
AFL_TEST("game.map.ShipPredictor:fuel-usage:host", a)
{
    testFuelUsage(a("1 ly -> 2 kt"),  1,  2);
    testFuelUsage(a("2 ly -> 5 kt"),  2,  5);
    testFuelUsage(a("3 ly -> 5 kt"),  3,  5);
    testFuelUsage(a("4 ly -> 11 kt"), 4, 11);
}

/** Test fuel usage computation for PHost, UseAccurateFuelModel.
    This checks the fuel prediction anomaly: before 4.0e/3.4h, it was close to impossible
    to end up with 0 fuel. */
AFL_TEST("game.map.ShipPredictor:fuel-usage:phost", a)
{
    testFuelUsagePHost(a("79 old"), 79, 78, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));
    testFuelUsagePHost(a("78 old"), 78, 79, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));

    testFuelUsagePHost(a("79 new"), 79, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
    testFuelUsagePHost(a("78 new"), 78, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
}

/** Test multiple cases of alchemy. */
AFL_TEST("game.map.ShipPredictor:alchemy", a)
{
    const HostVersion PHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion THOST = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    // Normal
    testAlchemy(a, "xyz", 900, 110, 120, 130,   0, PHOST, true,  false);
    testAlchemy(a, "xyz", 900, 110, 120, 130,   0, THOST, true,  false);
    testAlchemy(a, "xyz",  30,  13,  23,  33,   3, PHOST, true,  false);
    testAlchemy(a, "xyz",  30,  13,  23,  33,   3, THOST, true,  false);
    testAlchemy(a, "xyz",   0,  10,  20,  30,   0, PHOST, false, false);
    testAlchemy(a, "xyz",   0,  10,  20,  30,   0, THOST, false, false);

    // NAL
    testAlchemy(a, "NAL", 900,  10,  20,  30, 900, PHOST, false, true);
    testAlchemy(a, "NAL", 900,  10,  20,  30, 900, THOST, false, true);

    // alX
    testAlchemy(a, "alt", 900, 310,  20,  30,   0, PHOST, true,  true);
    testAlchemy(a, "alt", 900, 310,  20,  30,   0, THOST, true,  true);
    testAlchemy(a, "ald", 900,  10, 320,  30,   0, PHOST, true,  true);
    testAlchemy(a, "ald", 900,  10, 320,  30,   0, THOST, true,  true);
    testAlchemy(a, "alm", 900,  10,  20, 330,   0, PHOST, true,  true);
    testAlchemy(a, "alm", 900,  10,  20, 330,   0, THOST, true,  true);

    testAlchemy(a, "alt",  30,  20,  20,  30,   0, PHOST, true,  true);
    testAlchemy(a, "alt",  30,  20,  20,  30,   0, THOST, true,  true);
    // testAlchemy(a, "alt",  30,  19,  20,  30,   3, THOST, true,  true);

    // naX
    testAlchemy(a, "nat", 900,  10, 170, 180,   0, PHOST, true,  true);
    testAlchemy(a, "nat", 900, 110, 120, 130,   0, THOST, true,  false);
    testAlchemy(a, "nad", 900, 160,  20, 180,   0, PHOST, true,  true);
    testAlchemy(a, "nad", 900, 110, 120, 130,   0, THOST, true,  false);
    testAlchemy(a, "nam", 900, 160, 170,  30,   0, PHOST, true,  true);
    testAlchemy(a, "nam", 900, 110, 120, 130,   0, THOST, true,  false);
}

/** Test multiple cases of refinery. Note the PHost version dependency. */
AFL_TEST("game.map.ShipPredictor:refinery", a)
{
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));
    const HostVersion THOST    = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    static const int HULLFUNCS[] = { game::spec::BasicHullFunction::NeutronicRefinery };

    // Normal
    testGenericRefinery(a, "xyz", 900,  0,  0,  0, 840, 61, PHOST, true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz", 900,  0,  0,  0, 840, 61, THOST, true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz",  30,  0,  0, 30,   0, 31, PHOST, true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz",  30,  0,  0, 30,   0, 31, THOST, true,  false, HULLFUNCS);

    // NAL
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, PHOST, false, true, HULLFUNCS);
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, THOST, false, true, HULLFUNCS);

    // alX
    testGenericRefinery(a, "alt", 900,  0, 20, 30, 890, 11, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "ald", 900, 10,  0, 30, 880, 21, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "alm", 900, 10, 20,  0, 870, 31, PHOST, true,  true, HULLFUNCS);

    testGenericRefinery(a, "alt", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "ald", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "alm", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);

    testGenericRefinery(a, "alt", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "ald", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "alm", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);

    // naX
    testGenericRefinery(a, "nat", 900, 10,  0,  0, 850, 51, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "nad", 900,  0, 20,  0, 860, 41, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "nam", 900,  0,  0, 30, 870, 31, PHOST, true,  true, HULLFUNCS);

    testGenericRefinery(a, "nat", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nad", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nam", 900,  0,  0,  0, 840, 61, OLDPHOST, true, false, HULLFUNCS);

    testGenericRefinery(a, "nat", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nad", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nam", 900,  0,  0,  0, 840, 61, THOST, true, false, HULLFUNCS);
}

/** Test multiple cases of refinery. Note the PHost version dependency. */
AFL_TEST("game.map.ShipPredictor:advanced-refinery", a)
{
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));
    const HostVersion THOST    = HostVersion(HostVersion::Host,  MKVERSION(3,22,47));

    static const int HULLFUNCS[] = { game::spec::BasicHullFunction::AriesRefinery };

    // Normal
    testGenericRefinery(a, "xyz", 40,  0,  0,  0,  40, 61, PHOST, true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz", 40,  0,  0,  0,  40, 61, THOST, true,  false, HULLFUNCS);

    // NAL
    testGenericRefinery(a, "NAL", 40, 10, 20, 30,  40,  1, PHOST, false, true, HULLFUNCS);
    // testGenericRefinery(a, "NAL", 40,  0,  0,  0,  40, 61, THOST, true,  false, HULLFUNCS); <- FIXME: HOST does not permit NAL for Aries

    // alX
    testGenericRefinery(a, "alt", 40,  0, 20, 30,  40, 11, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "ald", 40, 10,  0, 30,  40, 21, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "alm", 40, 10, 20,  0,  40, 31, PHOST, true,  true, HULLFUNCS);

    testGenericRefinery(a, "alt", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "ald", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "alm", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);

    testGenericRefinery(a, "alt", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "ald", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "alm", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);

    // naX
    testGenericRefinery(a, "nat", 40, 10,  0,  0,  40, 51, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "nad", 40,  0, 20,  0,  40, 41, PHOST, true,  true, HULLFUNCS);
    testGenericRefinery(a, "nam", 40,  0,  0, 30,  40, 31, PHOST, true,  true, HULLFUNCS);

    testGenericRefinery(a, "nat", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nad", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nam", 40,  0,  0,  0,  40, 61, OLDPHOST, true, false, HULLFUNCS);

    testGenericRefinery(a, "nat", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nad", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);
    testGenericRefinery(a, "nam", 40,  0,  0,  0,  40, 61, THOST, true, false, HULLFUNCS);
}

/** Test multiple cases of 4:1 combined refinery. */
AFL_TEST("game.map.ShipPredictor:combined-refinery-4-to-1", a)
{
    // No need to do THost tests; THost does not have this kind of ships
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));

    static const int HULLFUNCS[] = { game::spec::BasicHullFunction::NeutronicRefinery,
                                     game::spec::BasicHullFunction::MerlinAlchemy };

    a.check("01. hasAlchemyCombinations", PHOST.hasAlchemyCombinations());
    a.check("02. hasAlchemyCombinations", !OLDPHOST.hasAlchemyCombinations());

    // Normal
    testGenericRefinery(a, "xyz", 900,  10,  20,  30,   0, 226, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz", 900, 110, 120, 130,   0,   1, OLDPHOST, true,  false, HULLFUNCS);

    // NAL
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, PHOST,    false, true, HULLFUNCS);
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, OLDPHOST, false, true, HULLFUNCS);

    // alX, naX has no effect for new PHost
    testGenericRefinery(a, "alt", 900,  10,  20,  30,   0, 226, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "alt", 900, 310,  20,  30,   0,   1, OLDPHOST, true,  true,  HULLFUNCS);
    testGenericRefinery(a, "nat", 900,  10,  20,  30,   0, 226, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "nat", 900,  10, 170, 180,   0,   1, OLDPHOST, true,  true,  HULLFUNCS);
}

/** Test multiple cases of 3:1 combined refinery. */
AFL_TEST("game.map.ShipPredictor:combined-refinery-3-to-1", a)
{
    // No need to do THost tests; THost does not have this kind of ships
    const HostVersion OLDPHOST = HostVersion(HostVersion::PHost, MKVERSION(4,0,5));
    const HostVersion PHOST    = HostVersion(HostVersion::PHost, MKVERSION(4,1,5));

    static const int HULLFUNCS[] = { game::spec::BasicHullFunction::AriesRefinery,
                                     game::spec::BasicHullFunction::MerlinAlchemy };

    a.check("01. hasAlchemyCombinations", PHOST.hasAlchemyCombinations());
    a.check("02. hasAlchemyCombinations", !OLDPHOST.hasAlchemyCombinations());

    // Normal
    testGenericRefinery(a, "xyz", 900,  10,  20,  30,   0, 301, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "xyz", 900, 110, 120, 130,   0,   1, OLDPHOST, true,  false, HULLFUNCS);

    // NAL
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, PHOST,    false, true, HULLFUNCS);
    testGenericRefinery(a, "NAL", 900, 10, 20, 30, 900,  1, OLDPHOST, false, true, HULLFUNCS);

    // alX, naX has no effect for new PHost
    testGenericRefinery(a, "alt", 900,  10,  20,  30,   0, 301, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "alt", 900, 310,  20,  30,   0,   1, OLDPHOST, true,  true,  HULLFUNCS);
    testGenericRefinery(a, "nat", 900,  10,  20,  30,   0, 301, PHOST,    true,  false, HULLFUNCS);
    testGenericRefinery(a, "nat", 900,  10, 170, 180,   0,   1, OLDPHOST, true,  true,  HULLFUNCS);
}

/*
 *  Movement
 */

const int MOVEMENT_SHIP_ID = 42;

// Base case
AFL_TEST("game.map.ShipPredictor:movement:normal", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 100);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", !p.isAtTurnLimit());
    a.check("02. isAtWaypoint", p.isAtWaypoint());
    a.checkEqual("03. getMovementFuelUsed", p.getMovementFuelUsed(), 41);
    a.checkEqual("04. getCloakFuelUsed", p.getCloakFuelUsed(), 0);
    a.check("05. isHyperdriving", !p.isHyperdriving());
}

// Timeout case (warp 1)
AFL_TEST("game.map.ShipPredictor:movement:timeout:warp1", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 100);
    s.setWaypoint(Point(X + 100, Y));
    s.setWarpFactor(1);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", p.isAtTurnLimit());
    a.check("02. isAtWaypoint", !p.isAtWaypoint());
    a.checkEqual("03. getMovementFuelUsed", p.getMovementFuelUsed(), 0);
    a.checkEqual("04. getPosition", p.getPosition(), Point(X + 30, Y));
    a.check("05. isHyperdriving", !p.isHyperdriving());
}

// Timeout case (warp 0)
AFL_TEST("game.map.ShipPredictor:movement:timeout:warp0", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 100);
    s.setWaypoint(Point(X + 100, Y));
    s.setWarpFactor(0);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", p.isAtTurnLimit());
    a.check("02. isAtWaypoint", !p.isAtWaypoint());
    a.checkEqual("03. getMovementFuelUsed", p.getMovementFuelUsed(), 0);
    a.checkEqual("04. getPosition", p.getPosition(), Point(X, Y));
    a.check("05. isHyperdriving", !p.isHyperdriving());
}

// Out of fuel
AFL_TEST("game.map.ShipPredictor:movement:out-of-fuel", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", !p.isAtTurnLimit());
    a.checkEqual("02. getMovementFuelUsed", p.getMovementFuelUsed(), 29);
    a.checkEqual("03. Neutronium", p.getCargo(Element::Neutronium), 0);
    a.check("04. isHyperdriving", !p.isHyperdriving());
}

// Out of fuel (2)
AFL_TEST("game.map.ShipPredictor:movement:out-of-fuel2", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.checkEqual("01. getMovementFuelUsed", p.getMovementFuelUsed(), 29);
    a.checkEqual("02. Neutronium", p.getCargo(Element::Neutronium), -19);
    a.check("03. isHyperdriving", !p.isHyperdriving());
}

// Training
AFL_TEST("game.map.ShipPredictor:movement:training", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);
    s.setMission(38, 0, 0);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.check("01. isAtWaypoint", !p.isAtWaypoint());
    a.check("02. isAtTurnLimit", !p.isAtTurnLimit());
    a.checkEqual("03. getMovementFuelUsed", p.getMovementFuelUsed(), 0);
    a.checkEqual("04. Neutronium", p.getCargo(Element::Neutronium), 10);
    a.checkEqual("05. getWarpFactor", p.getWarpFactor(), 0);
    a.checkEqual("06. UsedMission", p.getUsedProperties().contains(ShipPredictor::UsedMission), true);
    a.check("07. isHyperdriving", !p.isHyperdriving());
}

// Cloak
AFL_TEST("game.map.ShipPredictor:movement:cloak", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 100);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);
    s.addShipSpecialFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Cloak));
    s.setMission(game::spec::Mission::msn_Cloak, 0, 0);
    t.config[HostConfiguration::CloakFuelBurn].set(5);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", !p.isAtTurnLimit());
    a.check("02. isAtWaypoint", p.isAtWaypoint());
    a.checkEqual("03. getMovementFuelUsed", p.getMovementFuelUsed(), 40);
    a.checkEqual("04. getCloakFuelUsed", p.getCloakFuelUsed(), 10);
    a.check("05. isHyperdriving", !p.isHyperdriving());
}

// Gravitonic
AFL_TEST("game.map.ShipPredictor:movement:gravitonic", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, MOVEMENT_SHIP_ID);
    s.setCargo(Element::Neutronium, 480);
    s.addShipSpecialFunction(t.shipList.modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Gravitonic));
    s.setWaypoint(Point(X + 150, Y));
    s.setWarpFactor(9);

    ShipPredictor p(t.univ, MOVEMENT_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeMovement();

    a.check("01. isAtTurnLimit", !p.isAtTurnLimit());
    a.check("02. isAtWaypoint", p.isAtWaypoint());
    a.checkEqual("03. getNumTurns", p.getNumTurns(), 1);
    a.checkEqual("04. getMovementFuelUsed", p.getMovementFuelUsed(), 462);
    a.checkEqual("05. getCloakFuelUsed", p.getCloakFuelUsed(), 0);
    a.check("06. isHyperdriving", !p.isHyperdriving());
}


/** Test multiple cases of movement. */
AFL_TEST("game.map.ShipPredictor:movement:distances", a)
{
    const HostVersion HOST(HostVersion::Host, MKVERSION(3,22,40));

    // Test cases from http://phost.de/~stefan/movement.html
    // - Inexact, 4 quadrants
    testMovement2(a,  16,  82, 9,   16,  80, HOST);
    testMovement2(a, -16,  82, 9,  -16,  80, HOST);
    testMovement2(a,  16, -82, 9,   16, -80, HOST);
    testMovement2(a, -16, -82, 9,  -16, -80, HOST);

    testMovement2(a,  67,  53, 9,   64,  51, HOST);
    testMovement2(a, -67,  53, 9,  -64,  51, HOST);
    testMovement2(a,  67, -53, 9,   64, -51, HOST);
    testMovement2(a, -67, -53, 9,  -64, -51, HOST);

    // - exact, maximum distance
    testMovement2(a,  48,  66, 9,   48,  66, HOST);
    testMovement2(a, -48,  66, 9,  -48,  66, HOST);
    testMovement2(a,  48, -66, 9,   48, -66, HOST);
    testMovement2(a, -48, -66, 9,  -48, -66, HOST);

    // Some simple cases
    testMovement2(a,  10,  20, 5,   10,  20, HOST);
    testMovement2(a,  10,  20, 9,   10,  20, HOST);
    testMovement2(a, 100,   0, 4,   16,   0, HOST);
    testMovement2(a, 100,   0, 9,   81,   0, HOST);
}

/*
 *  Damage handling
 */

const int DAMAGE_SHIP_ID = 42;

// Damage speed limit
AFL_TEST("game.map.ShipPredictor:damage", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, DAMAGE_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);
    s.setDamage(50);

    ShipPredictor p(t.univ, DAMAGE_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.checkEqual("01. getWarpFactor", p.getWarpFactor(), 5);
    a.checkEqual("02. UsedDamageLimit", p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), true);
}

// Damage speed limit with self repair
AFL_TEST("game.map.ShipPredictor:damage:self-repair", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, DAMAGE_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setCargo(Element::Supplies, 102);     // fixes 20 damage -> 30 remaining
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);
    s.setDamage(50);

    ShipPredictor p(t.univ, DAMAGE_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.checkEqual("01. getWarpFactor", p.getWarpFactor(), 7);
    a.checkEqual("02. UsedDamageLimit", p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), true);
    a.checkEqual("03. UsedRepair", p.getUsedProperties().contains(ShipPredictor::UsedRepair), true);
    a.checkEqual("04. Supplies", p.getCargo(Element::Supplies), 2);
}

// Self repair, no damage limit
AFL_TEST("game.map.ShipPredictor:damage:self-repair:not-limited", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, DAMAGE_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setCargo(Element::Supplies, 400);     // fixes 80 damage
    s.setWaypoint(Point(X + 15, Y));
    s.setWarpFactor(9);
    s.setDamage(50);

    ShipPredictor p(t.univ, DAMAGE_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.checkEqual("01. getWarpFactor", p.getWarpFactor(), 9);
    a.checkEqual("02. UsedDamageLimit", p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), false);
    a.checkEqual("03. UsedRepair", p.getUsedProperties().contains(ShipPredictor::UsedRepair), true);
    a.checkEqual("04. Supplies", p.getCargo(Element::Supplies), 150);
}

// Base repair, no damage limit
AFL_TEST("game.map.ShipPredictor:damage:base-repair", a)
{
    TestHarness t;
    Ship& s = addEmerald(t, DAMAGE_SHIP_ID);
    s.setCargo(Element::Neutronium, 10);
    s.setCargo(Element::Supplies, 400);
    s.setWarpFactor(9);
    s.setDamage(50);

    game::map::Planet& b = *t.univ.planets().create(123);
    b.addCurrentPlanetData(game::map::PlanetData(), game::PlayerSet_t(1));
    b.addCurrentBaseData(game::map::BaseData(), game::PlayerSet_t(1));
    b.setPosition(Point(X, Y));
    b.setOwner(1);
    b.setBaseShipyardOrder(game::FixShipyardAction, DAMAGE_SHIP_ID);

    finish(t);

    ShipPredictor p(t.univ, DAMAGE_SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    p.computeTurn();

    a.checkEqual("01. getWarpFactor", p.getWarpFactor(), 9);
    a.checkEqual("02. UsedDamageLimit", p.getUsedProperties().contains(ShipPredictor::UsedDamageLimit), false);
    a.checkEqual("03. UsedRepair", p.getUsedProperties().contains(ShipPredictor::UsedRepair), false);
    a.checkEqual("04. UsedShipyard", p.getUsedProperties().contains(ShipPredictor::UsedShipyard), true);
    a.checkEqual("05. Supplies", p.getCargo(Element::Supplies), 400);
}

/*
 *  Torpedo related operations
 */

// mkt
AFL_TEST("game.map.ShipPredictor:mkt", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);

    Ship& s = addEmerald(t, SHIP_ID);
    s.setTorpedoType(6);
    s.setNumLaunchers(3);
    s.setCargo(Element::Tritanium, 20);
    s.setCargo(Element::Duranium, 15);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Money, 2000);
    s.setCargo(Element::fromTorpedoType(6), 3);
    s.setFriendlyCode(String_t("mkt"));

    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    a.checkEqual("01. Torpedoes",  p.getCargo(Element::fromTorpedoType(6)), 18);  // 15 built
    a.checkEqual("02. Tritanium",  p.getCargo(Element::Tritanium), 5);
    a.checkEqual("03. Duranium",   p.getCargo(Element::Duranium), 0);
    a.checkEqual("04. Molybdenum", p.getCargo(Element::Molybdenum), 15);
    a.checkEqual("05. Money",      p.getCargo(Element::Money), 2000 - 13*15);
    a.checkEqual("06. UsedFCode",  p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
}

// Lay Mines
AFL_TEST("game.map.ShipPredictor:lay-mines", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);

    Ship& s = addEmerald(t, SHIP_ID);
    s.setTorpedoType(6);
    s.setNumLaunchers(3);
    s.setCargo(Element::fromTorpedoType(6), 20);
    s.setFriendlyCode(String_t("mdh"));
    s.setMission(3, 0, 0);

    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    a.checkEqual("11. Torpedoes",   p.getCargo(Element::fromTorpedoType(6)), 10);  // 10 laid
    a.checkEqual("12. UsedFCode",   p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
    a.checkEqual("13. UsedMission", p.getUsedProperties().contains(ShipPredictor::UsedMission), true);
}

/*
 *  Fighter building
 */

// Fighter building
AFL_TEST("game.map.ShipPredictor:build-fighter:robot", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);

    Ship& s = addCarrier(t, SHIP_ID);
    s.setOwner(9);
    s.setMission(9, 0, 0);
    s.setCargo(Element::Tritanium,  30);
    s.setCargo(Element::Duranium,   30);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Supplies,   30);
    s.setCargo(Element::Fighters,   10);
    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    // 6 fighters built
    a.checkEqual("01. Tritanium",  p.getCargo(Element::Tritanium),  12);
    a.checkEqual("02. Duranium",   p.getCargo(Element::Duranium),   30);
    a.checkEqual("03. Molybdenum", p.getCargo(Element::Molybdenum), 18);
    a.checkEqual("04. Supplies",   p.getCargo(Element::Supplies),    0);
    a.checkEqual("05. Fighters",   p.getCargo(Element::Fighters),   16);

    a.checkEqual("11. UsedBuildFighters", p.getUsedProperties().contains(ShipPredictor::UsedBuildFighters), true);
}

// Fighter building, missing mineral
AFL_TEST("game.map.ShipPredictor:build-fighter:robot:missing", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);

    Ship& s = addCarrier(t, SHIP_ID);
    s.setOwner(9);
    s.setMission(9, 0, 0);
    s.setCargo(Element::Tritanium,   0);
    s.setCargo(Element::Duranium,   30);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Supplies,   30);
    s.setCargo(Element::Fighters,   10);
    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    // 6 fighters built
    a.checkEqual("01. Fighters", p.getCargo(Element::Fighters), 10);
    a.checkEqual("12. UsedBuildFighters", p.getUsedProperties().contains(ShipPredictor::UsedBuildFighters), false);
}

// Fighter building, unlimited due to zero cost
AFL_TEST("game.map.ShipPredictor:build-fighter:robot:unlimited", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);
    t.config[HostConfiguration::ShipFighterCost].set("S0");

    Ship& s = addCarrier(t, SHIP_ID);
    s.setOwner(9);
    s.setMission(9, 0, 0);
    s.setCargo(Element::Tritanium,  30);
    s.setCargo(Element::Duranium,   30);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Supplies,   30);
    s.setCargo(Element::Fighters,   10);
    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    // 6 fighters built
    a.checkEqual("01. Tritanium",  p.getCargo(Element::Tritanium),  30);
    a.checkEqual("02. Duranium",   p.getCargo(Element::Duranium),   30);
    a.checkEqual("03. Molybdenum", p.getCargo(Element::Molybdenum), 30);
    a.checkEqual("04. Supplies",   p.getCargo(Element::Supplies),   30);
    a.checkEqual("05. Fighters",   p.getCargo(Element::Fighters),  180);

    a.checkEqual("11. UsedBuildFighters", p.getUsedProperties().contains(ShipPredictor::UsedBuildFighters), true);
}

// Fighter building, limited by mission
AFL_TEST("game.map.ShipPredictor:build-fighter:robot:limited", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);
    t.config[HostConfiguration::ShipFighterCost].set("S0");

    Ship& s = addCarrier(t, SHIP_ID);
    s.setOwner(9);
    s.setMission(32, 17, 0);
    s.setCargo(Element::Tritanium,  30);
    s.setCargo(Element::Duranium,   30);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Supplies,   30);
    s.setCargo(Element::Fighters,   10);
    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    // 6 fighters built
    a.checkEqual("01. Tritanium",  p.getCargo(Element::Tritanium),  30);
    a.checkEqual("02. Duranium",   p.getCargo(Element::Duranium),   30);
    a.checkEqual("03. Molybdenum", p.getCargo(Element::Molybdenum), 30);
    a.checkEqual("04. Supplies",   p.getCargo(Element::Supplies),   30);
    a.checkEqual("05. Fighters",   p.getCargo(Element::Fighters),   27);

    a.checkEqual("11. UsedBuildFighters", p.getUsedProperties().contains(ShipPredictor::UsedBuildFighters), true);
    a.checkEqual("11. UsedMission",       p.getUsedProperties().contains(ShipPredictor::UsedMission),       true);
}

// Fighter building, rebel
AFL_TEST("game.map.ShipPredictor:build-fighter:rebel", a)
{
    const int SHIP_ID = 235;
    TestHarness t;
    game::test::initStandardTorpedoes(t.shipList);
    game::test::initStandardBeams(t.shipList);

    Ship& s = addCarrier(t, SHIP_ID);
    s.setOwner(10);
    s.setMission(1, 0, 0);
    s.setCargo(Element::Tritanium,  30);
    s.setCargo(Element::Duranium,   30);
    s.setCargo(Element::Molybdenum, 30);
    s.setCargo(Element::Supplies,   30);
    s.setCargo(Element::Fighters,   10);
    finish(t);

    game::test::RegistrationKey key(game::RegistrationKey::Registered, 10);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, key);
    p.computeTurn();

    // 6 fighters built
    a.checkEqual("01. Tritanium",  p.getCargo(Element::Tritanium),  12);
    a.checkEqual("02. Duranium",   p.getCargo(Element::Duranium),   30);
    a.checkEqual("03. Molybdenum", p.getCargo(Element::Molybdenum), 18);
    a.checkEqual("04. Supplies",   p.getCargo(Element::Supplies),    0);
    a.checkEqual("05. Fighters",   p.getCargo(Element::Fighters),   16);

    a.checkEqual("11. UsedBuildFighters", p.getUsedProperties().contains(ShipPredictor::UsedBuildFighters), true);
}

/*
 *  Others
 */

/** Test getOptimumWarp(). */
AFL_TEST("game.map.ShipPredictor:getOptimumWarp", a)
{
    const int SHIP_ID = 77;
    const int PLANET_X = 1300;
    const int PLANET_Y = 2400;

    TestHarness h;

    // Ship for testing: we don't care about the actual type,
    // but it needs to have a fully-specified engine.
    game::map::Ship& sh = addEmerald(h, SHIP_ID);
    sh.setEngineType(9);
    game::test::addTranswarp(h.shipList);

    // Add a planet for gravity tests
    h.univ.planets().create(100)->setPosition(Point(PLANET_X, PLANET_Y));
    finish(h);

    // Root.
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));

    // Test cases
    struct TestCase {
        int fromX, fromY;
        int toX, toY;
        int expect;
        const char* desc;
    };
    static const TestCase TESTS[] = {
        // Regular
        { 5000, 5000,             5000, 5080,             9,   "standard 1x warp 9 case" },
        { 5000, 5000,             5000, 5030,             6,   "standard 1x warp 6 case" },
        { 5000, 5000,             5000, 5090,             7,   "standard 2x warp 7 case" },

        // Starting in warp well
        { PLANET_X, PLANET_Y,     PLANET_X+10, PLANET_Y,  4,   "out of warp well" },
        { PLANET_X, PLANET_Y,     PLANET_X+1,  PLANET_Y,  1,   "inside warp well warp 1" },
        { PLANET_X, PLANET_Y,     PLANET_X+2,  PLANET_Y,  2,   "inside warp well warp 2" },

        // Starting outside warp well
        { PLANET_X+4, PLANET_Y,   PLANET_X+3, PLANET_Y,   2,   "into warp well" },
    };

    for (size_t i = 0; i < sizeof(TESTS)/sizeof(TESTS[0]); ++i) {
        const TestCase& c = TESTS[i];
        int result = getOptimumWarp(h.univ, SHIP_ID, Point(c.fromX, c.fromY), Point(c.toX, c.toY), h.shipScores, h.shipList, h.mapConfig, *root);
        a(c.desc).checkEqual("getOptimumWarp", result, c.expect);
    }
}

/** Test getOptimumWarp(), error cases. */

// Nonexistant ship
AFL_TEST("game.map.ShipPredictor:getOptimumWarp:no-ship", a)
{
    const int SHIP_ID = 77;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    TestHarness h;
    finish(h);
    int result = getOptimumWarp(h.univ, SHIP_ID, Point(1000, 1000), Point(1010, 1000), h.shipScores, h.shipList, h.mapConfig, *root);
    a.checkEqual("", result, 0);
}

// Nonexistant engine
AFL_TEST("game.map.ShipPredictor:getOptimumWarp:no-engine", a)
{
    const int SHIP_ID = 77;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    TestHarness h;
    game::map::Ship& sh = addEmerald(h, SHIP_ID);
    sh.setEngineType(9);
    finish(h);

    int result = getOptimumWarp(h.univ, SHIP_ID, Point(1000, 1000), Point(1010, 1000), h.shipScores, h.shipList, h.mapConfig, *root);
    a.checkEqual("", result, 0);
}

// Too far
AFL_TEST("game.map.ShipPredictor:getOptimumWarp:too-far", a)
{
    const int SHIP_ID = 77;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    TestHarness h;
    game::map::Ship& sh = addEmerald(h, SHIP_ID);
    game::test::addNovaDrive(h.shipList);
    sh.setEngineType(game::test::NOVA_ENGINE_ID);
    finish(h);

    int result = getOptimumWarp(h.univ, SHIP_ID, Point(1000, 1000), Point(1000 + 30*80, 1000), h.shipScores, h.shipList, h.mapConfig, *root);
    a.checkEqual("", result, 5);
}

// Hyperjump
AFL_TEST("game.map.ShipPredictor:getOptimumWarp:hyperjump", a)
{
    const int SHIP_ID = 77;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    TestHarness h;
    game::map::Ship& sh = addEmerald(h, SHIP_ID);
    game::test::addNovaDrive(h.shipList);
    sh.setEngineType(game::test::NOVA_ENGINE_ID);
    sh.setFriendlyCode(String_t("HYP"));
    sh.setWarpFactor(1);
    sh.addShipSpecialFunction(h.shipList.modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Hyperdrive));
    finish(h);

    int result = getOptimumWarp(h.univ, SHIP_ID, Point(1000, 1000), Point(1024, 1000), h.shipScores, h.shipList, h.mapConfig, *root);
    a.checkEqual("", result, 5);
}

/** Test hyperjump: regular jump. */
AFL_TEST("game.map.ShipPredictor:hyperjump:normal", a)
{
    const int SHIP_ID = 42;

    // Regular jump
    TestHarness t;
    t.hostVersion = HostVersion(HostVersion::PHost, MKVERSION(3,3,0));

    Ship& s = addJumper(t, SHIP_ID);
    s.setCargo(Element::Neutronium, 60);
    s.setWaypoint(Point(X + 20, Y));
    s.setWarpFactor(1);
    s.setFriendlyCode(String_t("HYP"));

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    a.check("01. isHyperdriving", p.isHyperdriving());
    p.computeTurn();

    a.checkEqual("11. getWarpFactor", p.getWarpFactor(), 0);     // reset by jump
    a.check("12. isHyperdriving", !p.isHyperdriving());             // no longer hyperdriving because speed was reset
    a.checkEqual("13. UsedFCode", p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
    a.checkEqual("14. getPosition", p.getPosition().getX(), X + 350);
    a.checkEqual("15. getPosition", p.getPosition().getY(), Y);
    a.checkEqual("16. Neutronium", p.getCargo(Element::Neutronium), 10);
}

/** Test hyperjump: direct (exact) jump. */
AFL_TEST("game.map.ShipPredictor:hyperjump:direct", a)
{
    const int SHIP_ID = 42;

    TestHarness t;
    t.hostVersion = HostVersion(HostVersion::PHost, MKVERSION(3,3,0));

    Ship& s = addJumper(t, SHIP_ID);
    s.setCargo(Element::Neutronium, 60);
    s.setWaypoint(Point(X + 10, Y + 340));
    s.setWarpFactor(1);
    s.setFriendlyCode(String_t("HYP"));

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    a.check("01. isHyperdriving", p.isHyperdriving());
    p.computeTurn();

    a.checkEqual("11. getWarpFactor", p.getWarpFactor(), 0);
    a.check("12. isHyperdriving", !p.isHyperdriving());             // no longer hyperdriving because speed was reset
    a.checkEqual("13. UsedFCode", p.getUsedProperties().contains(ShipPredictor::UsedFCode), true);
    a.checkEqual("14. getPosition", p.getPosition().getX(), X + 10);
    a.checkEqual("15. getPosition", p.getPosition().getY(), Y + 340);
    a.checkEqual("16. Neutronium", p.getCargo(Element::Neutronium), 10);
}

/** Test hyperjump: failure due to minimum distance violation. */
AFL_TEST("game.map.ShipPredictor:hyperjump:error:min-dist", a)
{
    const int SHIP_ID = 42;

    TestHarness t;
    t.hostVersion = HostVersion(HostVersion::Host, MKVERSION(3,2,0));

    Ship& s = addJumper(t, SHIP_ID);
    s.setCargo(Element::Neutronium, 60);
    s.setWaypoint(Point(X + 10, Y));
    s.setWarpFactor(1);
    s.setFriendlyCode(String_t("HYP"));

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    a.check("01. isHyperdriving", p.isHyperdriving());
    p.computeTurn();

    a.checkEqual("11. getWarpFactor", p.getWarpFactor(), 1);
    a.check("12. isHyperdriving", p.isHyperdriving());              // still trying to hyperjump
    a.checkEqual("13. UsedFCode", p.getUsedProperties().contains(ShipPredictor::UsedFCode), false);
    a.checkEqual("14. getPosition", p.getPosition().getX(), X + 1);
    a.checkEqual("15. getPosition", p.getPosition().getY(), Y);
    a.checkEqual("16. Neutronium", p.getCargo(Element::Neutronium), 60);
}

/** Test hyperjump: failure due to excess damage. */
AFL_TEST("game.map.ShipPredictor:hyperjump:error:damage", a)
{
    const int SHIP_ID = 42;

    TestHarness t;
    t.hostVersion = HostVersion(HostVersion::PHost, MKVERSION(3,3,0));
    t.config[HostConfiguration::DamageLevelForHyperjumpFail].set(15);

    Ship& s = addJumper(t, SHIP_ID);
    s.setCargo(Element::Neutronium, 60);
    s.setWaypoint(Point(X + 20, Y));
    s.setWarpFactor(1);
    s.setFriendlyCode(String_t("HYP"));
    s.setDamage(15);

    ShipPredictor p(t.univ, SHIP_ID, t.shipScores, t.shipList, t.mapConfig, t.config, t.hostVersion, t.key);
    a.check("01. isHyperdriving", p.isHyperdriving());
    p.computeTurn();

    a.checkEqual("11. getWarpFactor", p.getWarpFactor(), 1);
    a.check("12. isHyperdriving", p.isHyperdriving());              // still trying to hyperjump
    a.checkEqual("13. UsedFCode", p.getUsedProperties().contains(ShipPredictor::UsedFCode), false);
    a.checkEqual("14. getPosition", p.getPosition().getX(), X + 1);
    a.checkEqual("15. getPosition", p.getPosition().getY(), Y);
    a.checkEqual("16. Neutronium", p.getCargo(Element::Neutronium), 60);
}

/** Test computeMovementTime(), simple cases. */
AFL_TEST("game.map.ShipPredictor:computeMovementTime", a)
{
    game::map::Universe univ;
    game::map::Configuration config;
    afl::base::Ref<game::Root> root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))));

    // Move 80 ly in steps of 20, all cardinal directions
    a.checkEqual("01", game::map::computeMovementTime(Point(1000, 2000), Point(1000, 2080), 20, univ, config, *root), 4);
    a.checkEqual("02", game::map::computeMovementTime(Point(1000, 2000), Point(1000, 1920), 20, univ, config, *root), 4);
    a.checkEqual("03", game::map::computeMovementTime(Point(1000, 2000), Point(1080, 2000), 20, univ, config, *root), 4);
    a.checkEqual("04", game::map::computeMovementTime(Point(1000, 2000), Point( 920, 2000), 20, univ, config, *root), 4);

    // Move 0 ly
    a.checkEqual("11", game::map::computeMovementTime(Point(1000, 2000), Point(1000, 2000), 20, univ, config, *root), 0);

    // Move 300 ly in steps of 2 --> overrun
    a.checkEqual("21", game::map::computeMovementTime(Point(1000, 2000), Point(1000, 2300), 2, univ, config, *root), game::map::ShipPredictor::MOVEMENT_TIME_LIMIT);

    // Non-cardinal direction (slightly > 80 ly)
    a.checkEqual("31", game::map::computeMovementTime(Point(1000, 2000), Point(1020, 2080), 20, univ, config, *root), 4);

    // Original test case: Merah-5 (#461) to Albireo (#22) on Echo Cluster map
    a.checkEqual("41", game::map::computeMovementTime(Point(2164, 1277), Point(2078, 1418), 81, univ, config, *root), 3);
    a.checkEqual("42", game::map::computeMovementTime(Point(2164, 1277), Point(2078, 1417), 81, univ, config, *root), 2);
}

/** Test computeMovementTime(), with gravity at the end. Original test case. */
AFL_TEST("game.map.ShipPredictor:computeMovementTime:gravity-at-end", a)
{
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::map::Universe univ;
    game::map::Configuration config;
    afl::base::Ref<game::Root> root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))));

    game::map::Planet* pl = univ.planets().create(22);
    pl->setPosition(Point(2078, 1418));
    pl->internalCheck(config, game::PlayerSet_t(), 77, tx, log);

    // Original test case: Merah-5 (#461) to Albireo (#22) on Echo Cluster map, now with gravity
    a.checkEqual("", game::map::computeMovementTime(Point(2164, 1277), Point(2078, 1418), 81, univ, config, *root), 2);
}

/** Test computeMovementTime(), with gravity in the middle. Synthetic test case. */
AFL_TEST("game.map.ShipPredictor:computeMovementTime:gravity-in-middle", a)
{
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    game::map::Universe univ;
    game::map::Configuration config;
    afl::base::Ref<game::Root> root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))));

    game::map::Planet* pl = univ.planets().create(3);
    pl->setPosition(Point(2000, 1051));
    pl->internalCheck(config, game::PlayerSet_t(), 77, tx, log);

    // Cover 100 ly with 49 ly/turn. After first step, we end in the gravity of the planet at 1051.
    a.checkEqual("01", game::map::computeMovementTime(Point(2000, 1000), Point(2000, 1100), 49, univ, config, *root), 2);

    // We start the second turn from 1051, so we do not reach 1101 or farther.
    a.checkEqual("11", game::map::computeMovementTime(Point(2000, 1000), Point(2000, 1101), 49, univ, config, *root), 3);
    a.checkEqual("12", game::map::computeMovementTime(Point(2000, 1000), Point(2000, 1102), 49, univ, config, *root), 3);
}
