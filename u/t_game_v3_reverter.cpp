/**
  *  \file u/t_game_v3_reverter.cpp
  *  \brief Test for game::v3::Reverter
  */

#include <memory>
#include "game/v3/reverter.hpp"

#include "t_game_v3.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/game.hpp"
#include "game/map/locationreverter.hpp"
#include "game/ref/list.hpp"
#include "game/ref/sortbyid.hpp"
#include "game/turn.hpp"

using game::Element;
using game::PlayerSet_t;
using game::map::BaseData;
using game::map::Planet;
using game::map::PlanetData;
using game::map::Point;
using game::map::Ship;
using game::map::ShipData;

namespace {
    PlanetData makePlanet() {
        PlanetData pd;
        pd.minedNeutronium = 100;
        pd.minedTritanium  = 100;
        pd.minedDuranium   = 100;
        pd.minedMolybdenum = 100;
        pd.friendlyCode = "pfc";
        pd.colonistTax = 12;
        pd.owner       = 3;
        pd.colonistClans = 77;
        return pd;
    }

    ShipData makeShip() {
        ShipData sd;
        sd.neutronium = 20;
        sd.tritanium = 10;
        sd.duranium = 30;
        sd.molybdenum = 40;
        sd.friendlyCode = "sfc";
        sd.primaryEnemy = 3;
        sd.mission = 40;
        sd.missionTowParameter = 1;
        sd.missionInterceptParameter = 3;
        sd.x = 2000;
        sd.y = 2000;
        sd.owner = 3;
        sd.name = "ship 1";
        return sd;
    }
}


/** Test getPreviousShipFriendlyCode(), getPreviousPlanetFriendlyCode(). */
void
TestGameV3Reverter::testGetPreviousFriendlyCode()
{
    // Environment
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Testee
    game::v3::Reverter testee(turn, session);

    // Add some objects
    {
        ShipData sd;
        sd.friendlyCode = String_t("s20");
        testee.addShipData(20, sd);

        sd.friendlyCode = String_t("s30");
        testee.addShipData(30, sd);

        PlanetData pd;
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

/** Test getPreviousShipMission(). */
void
TestGameV3Reverter::testShipMission()
{
    // Environment
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Testee
    game::v3::Reverter testee(turn, session);

    // Add some objects
    {
        ShipData sd;
        sd.mission = 30;
        testee.addShipData(1, sd);              // partial ship -> not reporter

        sd.mission = 40;
        sd.missionInterceptParameter = 50;
        testee.addShipData(2, sd);              // partial ship -> not reporter

        sd.mission = 50;
        sd.missionInterceptParameter = 60;
        sd.missionTowParameter = 70;
        testee.addShipData(3, sd);              // complete ship -> will be reported
    }

    // Query
    int m, i, t;
    TS_ASSERT_EQUALS(testee.getPreviousShipMission(1, m, i, t), false);
    TS_ASSERT_EQUALS(testee.getPreviousShipMission(2, m, i, t), false);
    TS_ASSERT_EQUALS(testee.getPreviousShipMission(3, m, i, t), true);
    TS_ASSERT_EQUALS(m, 50);
    TS_ASSERT_EQUALS(i, 60);
    TS_ASSERT_EQUALS(t, 70);

    // Totally out-of-range should not crash
    TS_ASSERT(!testee.getPreviousShipMission(0, m, i, t));
    TS_ASSERT(!testee.getPreviousShipMission(22222, m, i, t));
    TS_ASSERT(!testee.getPreviousShipMission(-22222, m, i, t));
}

/** Test getMinBuildings(). */
void
TestGameV3Reverter::testMinBuildings()
{
    // Environment
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Testee
    game::v3::Reverter testee(turn, session);

    // Add some objects
    {
        PlanetData pd;
        pd.numFactories = 10;
        testee.addPlanetData(100, pd);

        pd.numFactories = 20;
        pd.numMines = 30;
        pd.numDefensePosts = 40;
        testee.addPlanetData(101, pd);

        BaseData bd;
        bd.numBaseDefensePosts = 50;
        testee.addBaseData(200, bd);
    }

    // Query
    TS_ASSERT_EQUALS(testee.getMinBuildings(100, game::MineBuilding).orElse(-1),        -1);
    TS_ASSERT_EQUALS(testee.getMinBuildings(100, game::DefenseBuilding).orElse(-1),     -1);
    TS_ASSERT_EQUALS(testee.getMinBuildings(100, game::FactoryBuilding).orElse(-1),     10);
    TS_ASSERT_EQUALS(testee.getMinBuildings(100, game::BaseDefenseBuilding).orElse(-1), -1);

    TS_ASSERT_EQUALS(testee.getMinBuildings(101, game::MineBuilding).orElse(-1),        30);
    TS_ASSERT_EQUALS(testee.getMinBuildings(101, game::DefenseBuilding).orElse(-1),     40);
    TS_ASSERT_EQUALS(testee.getMinBuildings(101, game::FactoryBuilding).orElse(-1),     20);
    TS_ASSERT_EQUALS(testee.getMinBuildings(101, game::BaseDefenseBuilding).orElse(-1), -1);

    TS_ASSERT_EQUALS(testee.getMinBuildings(200, game::MineBuilding).orElse(-1),        -1);
    TS_ASSERT_EQUALS(testee.getMinBuildings(200, game::DefenseBuilding).orElse(-1),     -1);
    TS_ASSERT_EQUALS(testee.getMinBuildings(200, game::FactoryBuilding).orElse(-1),     -1);
    TS_ASSERT_EQUALS(testee.getMinBuildings(200, game::BaseDefenseBuilding).orElse(-1), 50);

    // Totally out-of-range should not crash
    TS_ASSERT(!testee.getMinBuildings(20000, game::MineBuilding).isValid());
    TS_ASSERT(!testee.getMinBuildings(0,     game::MineBuilding).isValid());
    TS_ASSERT(!testee.getMinBuildings(-9999, game::MineBuilding).isValid());
}

/** Test createLocationReverter(), standard case.
    A: create ship and planet with undo information. Call createLocationReverter().
    E: units recognized for reset; reset operates correctly. */
void
TestGameV3Reverter::testLocation()
{
    // Environment
    afl::sys::Log log;
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::map::Universe& univ = turn.universe();

    // Add some units
    Planet& p1 = *univ.planets().create(77);
    Ship& s1 = *univ.ships().create(111);

    // Tester
    game::v3::Reverter testee(turn, session);

    // Populate planet
    {
        PlanetData pd = makePlanet();
        p1.setPosition(Point(2000, 2000));
        p1.addCurrentPlanetData(pd, PlayerSet_t(3));
        p1.setPlayability(Planet::Playable);
        p1.internalCheck(game::map::Configuration(), tx, log);

        PlanetData pd1 = pd;
        pd1.minedNeutronium = 20;
        pd1.friendlyCode = "ofc";
        testee.addPlanetData(p1.getId(), pd1);
    }

    // Populate ship
    {
        ShipData sd = makeShip();
        s1.addCurrentShipData(sd, PlayerSet_t(3));
        s1.setPlayability(Ship::Playable);
        s1.internalCheck();

        ShipData sd1 = sd;
        sd1.name = "old name";
        sd1.neutronium = 100;
        testee.addShipData(s1.getId(), sd1);
    }

    // Verify
    // - general
    std::auto_ptr<game::map::LocationReverter> rev(testee.createLocationReverter(Point(2000, 2000)));
    TS_ASSERT(rev.get() != 0);
    TS_ASSERT(rev->getAvailableModes().contains(game::map::LocationReverter::Cargo));
    TS_ASSERT(rev->getAvailableModes().contains(game::map::LocationReverter::Missions));

    // - unit list
    game::ref::List list = rev->getAffectedObjects();
    TS_ASSERT_EQUALS(list.size(), 2U);
    list.sort(game::ref::SortById());
    TS_ASSERT_EQUALS(list[0], game::Reference(game::Reference::Planet, 77));
    TS_ASSERT_EQUALS(list[1], game::Reference(game::Reference::Ship, 111));

    // Execute
    TS_ASSERT_THROWS_NOTHING(rev->commit(rev->getAvailableModes()));

    // Verify
    TS_ASSERT_EQUALS(p1.getFriendlyCode().orElse(""), "ofc");
    TS_ASSERT_EQUALS(p1.getCargo(Element::Neutronium).orElse(0), 20);
    TS_ASSERT_EQUALS(s1.getName(), "old name");
    TS_ASSERT_EQUALS(s1.getCargo(Element::Neutronium).orElse(0), 100);
}

/** Test createLocationReverter(), empty case.
    A: Call createLocationReverter() on empty universe.
    E: If reverter is created, it reports no objects. */
void
TestGameV3Reverter::testLocationEmpty()
{
    // Environment
    afl::sys::Log log;
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);         // required for shiplist and root for prepareUndoInformation, i.e. not needed here

    // Tester
    game::v3::Reverter testee(turn, session);

    // Verify
    std::auto_ptr<game::map::LocationReverter> rev(testee.createLocationReverter(Point(2000, 2000)));
    if (rev.get() != 0) {
        TS_ASSERT_EQUALS(rev->getAffectedObjects().size(), 0U);
        TS_ASSERT_THROWS_NOTHING(rev->commit(rev->getAvailableModes()));
    }
}

/** Test createLocationReverter(), half-initialized case.
    A: create ship and planet, but only one has undo information. Call createLocationReverter().
    E: Reverter must not allow undo of Cargo, and not list the object without undo information. */
void
TestGameV3Reverter::testLocationHalf()
{
    // Environment
    afl::sys::Log log;
    game::Turn turn;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    game::map::Universe& univ = turn.universe();

    // Add some units
    Planet& p1 = *univ.planets().create(77);
    Ship& s1 = *univ.ships().create(111);

    // Tester
    game::v3::Reverter testee(turn, session);

    // Populate planet
    {
        PlanetData pd = makePlanet();
        p1.setPosition(Point(2000, 2000));
        p1.addCurrentPlanetData(pd, PlayerSet_t(3));
        p1.setPlayability(Planet::Playable);
        p1.internalCheck(game::map::Configuration(), tx, log);

        PlanetData pd1 = pd;
        pd1.minedNeutronium = 20;
        pd1.friendlyCode = "ofc";
        testee.addPlanetData(p1.getId(), pd1);
    }

    // Populate ship but give it no undo data
    {
        ShipData sd = makeShip();
        s1.addCurrentShipData(sd, PlayerSet_t(3));
        s1.setPlayability(Ship::Playable);
        s1.internalCheck();
    }

    // Verify
    // - general
    std::auto_ptr<game::map::LocationReverter> rev(testee.createLocationReverter(Point(2000, 2000)));
    TS_ASSERT(rev.get() != 0);
    TS_ASSERT(!rev->getAvailableModes().contains(game::map::LocationReverter::Cargo));
    TS_ASSERT(rev->getAvailableModes().contains(game::map::LocationReverter::Missions));

    // - unit list contains only planet
    game::ref::List list = rev->getAffectedObjects();
    TS_ASSERT_EQUALS(list.size(), 1U);
    TS_ASSERT_EQUALS(list[0], game::Reference(game::Reference::Planet, 77));

    // Execute
    TS_ASSERT_THROWS_NOTHING(rev->commit(rev->getAvailableModes()));

    // Verify
    TS_ASSERT_EQUALS(p1.getFriendlyCode().orElse(""), "ofc");
    TS_ASSERT_EQUALS(p1.getCargo(Element::Neutronium).orElse(0), 100);
    TS_ASSERT_EQUALS(s1.getName(), "ship 1");
    TS_ASSERT_EQUALS(s1.getCargo(Element::Neutronium).orElse(0), 20);
}

