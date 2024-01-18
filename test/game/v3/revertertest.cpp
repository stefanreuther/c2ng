/**
  *  \file test/game/v3/revertertest.cpp
  *  \brief Test for game::v3::Reverter
  */

#include "game/v3/reverter.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/locationreverter.hpp"
#include "game/ref/list.hpp"
#include "game/ref/sortby.hpp"
#include "game/turn.hpp"
#include <memory>

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
AFL_TEST("game.v3.Reverter:previous-fcode", a)
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
    a.checkEqual("01", testee.getPreviousShipFriendlyCode(10).orElse("x"), "x");
    a.checkEqual("02", testee.getPreviousShipFriendlyCode(20).orElse("x"), "s20");
    a.checkEqual("03", testee.getPreviousShipFriendlyCode(30).orElse("x"), "s30");
    a.checkEqual("04", testee.getPreviousShipFriendlyCode(40).orElse("x"), "x");

    a.checkEqual("11", testee.getPreviousPlanetFriendlyCode(10).orElse("x"), "x");
    a.checkEqual("12", testee.getPreviousPlanetFriendlyCode(20).orElse("x"), "x");
    a.checkEqual("13", testee.getPreviousPlanetFriendlyCode(30).orElse("x"), "p30");
    a.checkEqual("14", testee.getPreviousPlanetFriendlyCode(40).orElse("x"), "p40");

    // Totally out-of-range should not crash
    a.check("21", !testee.getPreviousPlanetFriendlyCode(0).isValid());
    a.check("22", !testee.getPreviousPlanetFriendlyCode(22222).isValid());
    a.check("23", !testee.getPreviousPlanetFriendlyCode(-22222).isValid());
}

/** Test getPreviousShipMission(). */
AFL_TEST("game.v3.Reverter:getPreviousShipMission", a)
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
    a.checkEqual("01. getPreviousShipMission", testee.getPreviousShipMission(1, m, i, t), false);
    a.checkEqual("02. getPreviousShipMission", testee.getPreviousShipMission(2, m, i, t), false);
    a.checkEqual("03. getPreviousShipMission", testee.getPreviousShipMission(3, m, i, t), true);
    a.checkEqual("04. m", m, 50);
    a.checkEqual("05. i", i, 60);
    a.checkEqual("06. t", t, 70);

    // Totally out-of-range should not crash
    a.check("11. getPreviousShipMission", !testee.getPreviousShipMission(0, m, i, t));
    a.check("12. getPreviousShipMission", !testee.getPreviousShipMission(22222, m, i, t));
    a.check("13. getPreviousShipMission", !testee.getPreviousShipMission(-22222, m, i, t));
}

/** Test getMinBuildings(). */
AFL_TEST("game.v3.Reverter:getMinBuildings", a)
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
    a.checkEqual("01", testee.getMinBuildings(100, game::MineBuilding).orElse(-1),        -1);
    a.checkEqual("02", testee.getMinBuildings(100, game::DefenseBuilding).orElse(-1),     -1);
    a.checkEqual("03", testee.getMinBuildings(100, game::FactoryBuilding).orElse(-1),     10);
    a.checkEqual("04", testee.getMinBuildings(100, game::BaseDefenseBuilding).orElse(-1), -1);

    a.checkEqual("11", testee.getMinBuildings(101, game::MineBuilding).orElse(-1),        30);
    a.checkEqual("12", testee.getMinBuildings(101, game::DefenseBuilding).orElse(-1),     40);
    a.checkEqual("13", testee.getMinBuildings(101, game::FactoryBuilding).orElse(-1),     20);
    a.checkEqual("14", testee.getMinBuildings(101, game::BaseDefenseBuilding).orElse(-1), -1);

    a.checkEqual("21", testee.getMinBuildings(200, game::MineBuilding).orElse(-1),        -1);
    a.checkEqual("22", testee.getMinBuildings(200, game::DefenseBuilding).orElse(-1),     -1);
    a.checkEqual("23", testee.getMinBuildings(200, game::FactoryBuilding).orElse(-1),     -1);
    a.checkEqual("24", testee.getMinBuildings(200, game::BaseDefenseBuilding).orElse(-1), 50);

    // Totally out-of-range should not crash
    a.check("31", !testee.getMinBuildings(20000, game::MineBuilding).isValid());
    a.check("32", !testee.getMinBuildings(0,     game::MineBuilding).isValid());
    a.check("33", !testee.getMinBuildings(-9999, game::MineBuilding).isValid());
}

/** Test createLocationReverter(), standard case.
    A: create ship and planet with undo information. Call createLocationReverter().
    E: units recognized for reset; reset operates correctly. */
AFL_TEST("game.v3.Reverter:createLocationReverter:normal", a)
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
        p1.internalCheck(game::map::Configuration(), PlayerSet_t(3), 15, tx, log);

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
        s1.internalCheck(PlayerSet_t(3), 15);

        ShipData sd1 = sd;
        sd1.name = "old name";
        sd1.neutronium = 100;
        testee.addShipData(s1.getId(), sd1);
    }

    // Verify
    // - general
    std::auto_ptr<game::map::LocationReverter> rev(testee.createLocationReverter(Point(2000, 2000)));
    a.checkNonNull("01. createLocationReverter", rev.get());
    a.check("02. Cargo",    rev->getAvailableModes().contains(game::map::LocationReverter::Cargo));
    a.check("03. Missions", rev->getAvailableModes().contains(game::map::LocationReverter::Missions));

    // - unit list
    game::ref::List list = rev->getAffectedObjects();
    a.checkEqual("11. size", list.size(), 2U);
    list.sort(game::ref::SortBy::Id());
    a.checkEqual("12. list", list[0], game::Reference(game::Reference::Planet, 77));
    a.checkEqual("13. list", list[1], game::Reference(game::Reference::Ship, 111));

    // Execute
    AFL_CHECK_SUCCEEDS(a("21. commit"), rev->commit(rev->getAvailableModes()));

    // Verify
    a.checkEqual("31. getFriendlyCode", p1.getFriendlyCode().orElse(""), "ofc");
    a.checkEqual("32. Neutronium",      p1.getCargo(Element::Neutronium).orElse(0), 20);
    a.checkEqual("33. getName",         s1.getName(), "old name");
    a.checkEqual("34. Neutronium",      s1.getCargo(Element::Neutronium).orElse(0), 100);
}

/** Test createLocationReverter(), empty case.
    A: Call createLocationReverter() on empty universe.
    E: If reverter is created, it reports no objects. */
AFL_TEST("game.v3.Reverter:createLocationReverter:empty", a)
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
        a.checkEqual("01. getAffectedObjects", rev->getAffectedObjects().size(), 0U);
        AFL_CHECK_SUCCEEDS(a("02. commit"), rev->commit(rev->getAvailableModes()));
    }
}

/** Test createLocationReverter(), half-initialized case.
    A: create ship and planet, but only one has undo information. Call createLocationReverter().
    E: Reverter must not allow undo of Cargo, and not list the object without undo information. */
AFL_TEST("game.v3.Reverter:createLocationReverter:partial", a)
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
        p1.internalCheck(game::map::Configuration(), PlayerSet_t(3), 15, tx, log);

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
        s1.internalCheck(PlayerSet_t(3), 15);
    }

    // Verify
    // - general
    std::auto_ptr<game::map::LocationReverter> rev(testee.createLocationReverter(Point(2000, 2000)));
    a.checkNonNull("01. createLocationReverter", rev.get());
    a.check("02. Cargo",   !rev->getAvailableModes().contains(game::map::LocationReverter::Cargo));
    a.check("03. Missions", rev->getAvailableModes().contains(game::map::LocationReverter::Missions));

    // - unit list contains only planet
    game::ref::List list = rev->getAffectedObjects();
    a.checkEqual("11. size", list.size(), 1U);
    a.checkEqual("12. list", list[0], game::Reference(game::Reference::Planet, 77));

    // Execute
    AFL_CHECK_SUCCEEDS(a("21. commit"), rev->commit(rev->getAvailableModes()));

    // Verify
    a.checkEqual("31. getFriendlyCode", p1.getFriendlyCode().orElse(""), "ofc");
    a.checkEqual("32. Neutronium",      p1.getCargo(Element::Neutronium).orElse(0), 100);
    a.checkEqual("33. getName",         s1.getName(), "ship 1");
    a.checkEqual("34. Neutronium",      s1.getCargo(Element::Neutronium).orElse(0), 20);
}
