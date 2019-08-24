/**
  *  \file u/t_game_map_shippredictor.cpp
  *  \brief Test for game::map::ShipPredictor
  */

#include "game/map/shippredictor.hpp"

#include "t_game_map.hpp"
#include "game/map/universe.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/test/registrationkey.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/engine.hpp"
#include "afl/string/nulltranslator.hpp"

namespace {
    using game::HostVersion;
    using game::map::Point;
    using game::Element;
    using game::config::HostConfiguration;

    const int X = 1200;
    const int Y = 1300;

    struct TestHarness {
        game::map::Universe univ;
        game::UnitScoreDefinitionList shipScores;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion hostVersion;
        game::test::RegistrationKey key;

        TestHarness()
            : univ(), shipScores(), shipList(), config(), hostVersion(), key(game::RegistrationKey::Unknown, 6)
            { }
    };

    game::map::Ship* addEmerald(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Emerald (from game "Schule", turn 61, ship #348)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 6;

        // Emerald:
        game::spec::Hull* h = t.shipList.hulls().create(HULL_ID);
        h->setMaxFuel(480);
        h->setMaxCargo(510);
        h->setMaxCrew(258);
        h->setNumEngines(2);
        h->setMass(218);        // we'll not add weapons; the plain hull only weighs 180 kt

        // HeavyNovaDrive 6:
        game::spec::Engine* e = t.shipList.engines().create(ENGINE_ID);
        e->setFuelFactor(9, 72900);

        // Add a ship
        // - required properties
        game::map::Ship* s = t.univ.ships().create(shipId);
        s->addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s->setOwner(1);
        s->setHull(HULL_ID);
        s->setEngineType(ENGINE_ID);
        s->setPosition(game::map::Point(X, Y));
        s->setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s->setBeamType(0);
        s->setNumBeams(0);
        s->setTorpedoType(0);
        s->setNumLaunchers(0);
        s->setNumBays(0);
        s->setCargo(Element::Neutronium, 100);
        s->setCargo(Element::Tritanium, 0);
        s->setCargo(Element::Duranium, 0);
        s->setCargo(Element::Molybdenum, 0);
        s->setCargo(Element::Supplies, 0);
        s->setCargo(Element::Money, 0);
        s->setCargo(Element::Colonists, 0);
        s->setAmmo(0);

        return s;
    }

    game::map::Ship* addMerlin(TestHarness& t, int shipId)
    {
        /*
         *  Test case: Merlin (from game "qvs0", turn 110, ship #2)
         */
        const int HULL_ID = 61;
        const int ENGINE_ID = 9;

        // Emerald:
        game::spec::Hull* h = t.shipList.hulls().create(HULL_ID);
        h->setMaxFuel(450);
        h->setMaxCargo(2700);
        h->setMaxCrew(120);
        h->setNumEngines(10);
        h->setMass(928);        // we'll not add weapons; the plain hull only weighs 920 kt

        // Transwarp Drive:
        game::spec::Engine* e = t.shipList.engines().create(ENGINE_ID);
        e->setFuelFactor(9, 8100);

        // Add a ship
        // - required properties
        game::map::Ship* s = t.univ.ships().create(shipId);
        s->addCurrentShipData(game::map::ShipData(), game::PlayerSet_t(1));
        s->setOwner(1);
        s->setHull(HULL_ID);
        s->setEngineType(ENGINE_ID);
        s->setPosition(game::map::Point(X, Y));
        s->setWarpFactor(9);

        // - types and cargo need to be set to be able to compute a mass
        s->setBeamType(0);
        s->setNumBeams(0);
        s->setTorpedoType(0);
        s->setNumLaunchers(0);
        s->setNumBays(0);
        s->setCargo(Element::Neutronium, 100);
        s->setCargo(Element::Tritanium, 0);
        s->setCargo(Element::Duranium, 0);
        s->setCargo(Element::Molybdenum, 0);
        s->setCargo(Element::Supplies, 0);
        s->setCargo(Element::Money, 0);
        s->setCargo(Element::Colonists, 0);
        s->setAmmo(0);

        return s;
    }

    void finish(TestHarness& t)
    {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        t.univ.postprocess(game::PlayerSet_t::allUpTo(11),      // playingSet
                           game::PlayerSet_t::allUpTo(11),      // availablePlayers
                           game::map::Object::Editable,         // playability
                           t.hostVersion,                       // host
                           t.config,                            // config
                           42,                                  // turn
                           t.shipList,                          // shipList
                           tx,                                  // translator
                           log);                                // log
    }

    /* Canned test case: ship moving X light years burns Y kt fuel
       (THost distance anomaly) */
    void testFuelUsage(const char* label, int distance, int expected)
    {
        const int SHIP_ID = 348;

        TestHarness t;
        t.hostVersion = HostVersion(HostVersion::Host, MKVERSION(3,22,0));
        game::map::Ship* s = addEmerald(t, SHIP_ID);
        s->setWaypoint(Point(X + distance, Y));

        finish(t);

        game::map::ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getMovementFuelUsed(), expected);
    }

    /* Canned test case: ship having HAVE fuel needs NEED
       (PHost fuel consumption anomaly) */
    void testFuelUsagePHost(const char* label, int have, int need, HostVersion version)
    {
        const int SHIP_ID = 2;

        TestHarness t;
        t.hostVersion = version;
        game::map::Ship* s = addMerlin(t, SHIP_ID);
        s->setWaypoint(Point(X + 75, Y + 34));
        s->setCargo(Element::Neutronium, have);
        t.config[HostConfiguration::UseAccurateFuelModel].set(true);

        finish(t);

        game::map::ShipPredictor testee(t.univ, SHIP_ID, t.shipScores, t.shipList, t.config, t.hostVersion, t.key);
        testee.computeTurn();

        TSM_ASSERT_EQUALS(label, testee.getMovementFuelUsed(), need);
    }
}

/** Test fuel usage computation for THost.
    This checks the distance computation anomaly: a ship moving 3.00 ly burns the same
    amount of fuel as one moving 2.00 ly. */
void
TestGameMapShipPredictor::testFuelUsageHost()
{
    ::testFuelUsage("1 ly -> 2 kt",  1,  2);
    ::testFuelUsage("2 ly -> 5 kt",  2,  5);
    ::testFuelUsage("3 ly -> 5 kt",  3,  5);
    ::testFuelUsage("4 ly -> 11 kt", 4, 11);
}

/** Test fuel usage computation for PHost, UseAccurateFuelModel.
    This checks the fuel prediction anomaly: before 4.0e/3.4h, it was close to impossible
    to end up with 0 fuel. */
void
TestGameMapShipPredictor::testFuelUsagePHost()
{
    ::testFuelUsagePHost("79 old", 79, 78, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));
    ::testFuelUsagePHost("78 old", 78, 79, HostVersion(HostVersion::PHost, MKVERSION(3,2,5)));

    ::testFuelUsagePHost("79 new", 79, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
    ::testFuelUsagePHost("78 new", 78, 78, HostVersion(HostVersion::PHost, MKVERSION(4,0,5)));
}

