/**
  *  \file test/game/map/shipinfotest.cpp
  *  \brief Test for game::map::ShipInfo
  */

#include "game/map/shipinfo.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include <algorithm>

using game::map::Point;
using game::map::Ship;
using game::map::ShipMovementInfo;
using game::spec::Hull;

namespace {
    struct Environment {
        util::NumberFormatter fmt;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;

        Environment()
            : fmt(true, true), shipList(), tx()
            { }
    };

    struct MoveEnvironment {
        game::map::Universe univ;
        game::UnitScoreDefinitionList shipScores;
        game::map::Configuration mapConfig;
        game::TeamSettings teams;
        game::spec::ShipList shipList;
        afl::base::Ref<game::Root> root;

        MoveEnvironment()
            : univ(), shipScores(), mapConfig(), teams(), shipList(), root(game::test::makeRoot(game::HostVersion()))
            {
                game::test::addAnnihilation(shipList);
            }
    };

    void addShipTrack(Ship& ship, int turnNr, Point pos)
    {
        game::parser::MessageInformation mi(game::parser::MessageInformation::Ship, ship.getId(), turnNr);
        mi.addValue(game::parser::mi_X, pos.getX());
        mi.addValue(game::parser::mi_Y, pos.getY());
        mi.addValue(game::parser::mi_Mass, 100);
        ship.addMessageInformation(mi, game::PlayerSet_t());
    }

    Ship& addShip(afl::test::Assert a, MoveEnvironment& env, int id, Point pos)
    {
        Ship* sh = env.univ.ships().create(id);
        a.checkNonNull("ship created", sh);

        game::map::ShipData sd;
        sd.owner = 4;
        sd.x = pos.getX();
        sd.y = pos.getY();
        sd.hullType = game::test::ANNIHILATION_HULL_ID;
        sh->addCurrentShipData(sd, game::PlayerSet_t(4));
        sh->internalCheck(game::PlayerSet_t(4), /* turn: */ 15);
        sh->setPlayability(Ship::Playable);
        return *sh;
    }

    bool hasInfo(const game::map::ShipMovementInfos_t& result, const ShipMovementInfo& ele)
    {
        return std::find(result.begin(), result.end(), ele) != result.end();
    }
}

/** Test packShipLocationInfo(). */
AFL_TEST("game.map.ShipInfo:packShipLocationInfo", a)
{
    const int HULL_NR = 30;
    const int TURN_NR = 5;
    const Point PA(1000, 1000);
    const Point PB(1100, 1100);
    const Point PC(1150, 1100);

    // Misc environment
    game::map::Configuration mapConfig;
    game::config::HostConfiguration config;
    game::HostVersion host;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Ship list
    game::spec::ShipList sl;
    sl.engines().create(9);
    sl.hulls().create(HULL_NR)->setMass(400);

    // Universe
    game::map::Universe univ;
    game::map::Planet* pl = univ.planets().create(99);
    pl->setPosition(PB);
    pl->setName("Pluto");
    pl->internalCheck(mapConfig, game::PlayerSet_t(), TURN_NR, tx, log);

    // Create a ship
    Ship sh(33);
    sh.setHull(HULL_NR);
    sh.setEngineType(9);
    sh.addShipXYData(PA, 3, 300, game::PlayerSet_t(10));
    sh.setWaypoint(Point(1000, 900));
    sh.setWarpFactor(3);
    addShipTrack(sh, TURN_NR-1, PB);
    addShipTrack(sh, TURN_NR-2, PC);
    sh.internalCheck(game::PlayerSet_t(10), TURN_NR);

    // Do it
    game::map::ShipLocationInfos_t result;
    packShipLocationInfo(result, sh, univ, TURN_NR, mapConfig, config, host, sl, tx);

    // Verify
    a.check("01. result size", result.size() >= 3);
    a.check("02. result size", result.size() <= TURN_NR);

    a.checkEqual("11. result[0]", result[0].turnNumber, TURN_NR);
    a.checkEqual("12. result[0]", result[0].position.isValid(), true);
    a.checkEqual("13. result[0]", result[0].position.get()->getX(), 1000);
    a.checkEqual("14. result[0]", result[0].position.get()->getY(), 1000);
    a.checkEqual("15. result[0]", result[0].positionName, "(1000,1000)");
    a.checkEqual("16. result[0]", result[0].mass.orElse(-1), 300);            // from shipxy, because it is a scanned ship
    a.checkEqual("17. result[0]", result[0].heading.orElse(-1), 180);         // actual angle, not from history
    a.checkEqual("18. result[0]", result[0].warpFactor.orElse(-1), 3);
    a.checkEqual("19. result[0]", int(result[0].distanceMoved.orElse(-1)), 141);

    a.checkEqual("21. result[1]", result[1].turnNumber, TURN_NR-1);
    a.checkEqual("22. result[1]", result[1].position.isValid(), true);
    a.checkEqual("23. result[1]", result[1].position.get()->getX(), 1100);
    a.checkEqual("24. result[1]", result[1].position.get()->getY(), 1100);
    a.checkEqual("25. result[1]", result[1].positionName, "Orbit of Pluto (#99)");
    a.checkEqual("26. result[1]", result[1].mass.orElse(-1), 100);            // from history
    a.checkEqual("27. result[1]", result[1].heading.isValid(), false);        // from history
    a.checkEqual("28. result[1]", result[1].warpFactor.isValid(), false);
    a.checkEqual("29. result[1]", int(result[1].distanceMoved.orElse(-1)), 50);

    a.checkEqual("31. result[2]", result[2].turnNumber, TURN_NR-2);
    a.checkEqual("32. result[2]", result[2].position.isValid(), true);
    a.checkEqual("33. result[2]", result[2].position.get()->getX(), 1150);
    a.checkEqual("34. result[2]", result[2].position.get()->getY(), 1100);
    a.checkEqual("35. result[2]", result[2].positionName, "(1150,1100)");
    a.checkEqual("36. result[2]", result[2].mass.orElse(-1), 100);            // from history
    a.checkEqual("37. result[2]", result[2].heading.isValid(), false);        // from history
    a.checkEqual("38. result[2]", result[2].warpFactor.isValid(), false);
    a.checkEqual("39. result[2]", result[2].distanceMoved.isValid(), false);

    if (result.size() > 3) {
        a.checkEqual("41. result[3]", result[3].turnNumber, TURN_NR-3);
        a.checkEqual("42. result[3]", result[3].position.isValid(), false);
    }
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:empty", a)
{
    // Given a ship with unknown hull...
    Environment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 400, game::PlayerSet_t(4));
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect no result.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);
    a.checkEqual("01", result.size(), 0U);
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:freighter", a)
{
    // Given a freighter hull with 300 kt...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMass(300);
    h->setMaxCargo(200);
    h->setMaxFuel(150);

    // ...and a ship with 400 kt...
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 400, game::PlayerSet_t(4));
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect a valid result: scanned mass indicates neither tank, nor cargo hold are full, but they might be empty.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Current mass: 400 kt");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "Hull");
    a.checkEqual("05. result", result[1].value, "300");
    a.checkEqual("06. result", result[1].unit, "kt");
    a.checkEqual("07. result", result[2].name, "Cargo+Fuel");
    a.checkEqual("08. result", result[2].value, "100");
    a.checkEqual("09. result", result[2].unit, "kt");
    a.checkEqual("10. result", result[3].name, "\xE2\x96\xB6 Max. Fuel");
    a.checkEqual("11. result", result[3].value, "100");
    a.checkEqual("12. result", result[3].unit, "kt");
    a.checkEqual("13. result", result[4].name, "\xE2\x96\xB6 Max. Cargo");
    a.checkEqual("14. result", result[4].value, "100");
    a.checkEqual("15. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:freighter2", a)
{
    // Given a freighter hull with 300 kt...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMass(300);
    h->setMaxCargo(200);
    h->setMaxFuel(150);

    // ...and a ship with 510 kt...
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect a valid result: scanned mass indicates neither tank, nor cargo hold are empty, but they might be full.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Current mass: 510 kt");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "Hull");
    a.checkEqual("05. result", result[1].value, "300");
    a.checkEqual("06. result", result[1].unit, "kt");
    a.checkEqual("07. result", result[2].name, "Cargo+Fuel");
    a.checkEqual("08. result", result[2].value, "210");
    a.checkEqual("09. result", result[2].unit, "kt");
    a.checkEqual("10. result", result[3].name, "\xE2\x96\xB6 Min. Fuel");
    a.checkEqual("11. result", result[3].value, "10");
    a.checkEqual("12. result", result[3].unit, "kt");
    a.checkEqual("13. result", result[4].name, "\xE2\x96\xB6 Min. Cargo");
    a.checkEqual("14. result", result[4].value, "60");
    a.checkEqual("15. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:freighter3", a)
{
    // Given a freighter hull with 300 kt...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMass(300);
    h->setMaxCargo(2000);
    h->setMaxFuel(150);

    // ...and a ship with 510 kt...
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect a valid result: scanned mass indicates fuel tank might be full or empty, cargo follows from that
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Current mass: 510 kt");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "Hull");
    a.checkEqual("05. result", result[1].value, "300");
    a.checkEqual("06. result", result[1].unit, "kt");
    a.checkEqual("07. result", result[2].name, "Cargo+Fuel");
    a.checkEqual("08. result", result[2].value, "210");
    a.checkEqual("09. result", result[2].unit, "kt");
    a.checkEqual("10. result", result[3].name, "\xE2\x96\xB6 Max. Cargo");
    a.checkEqual("11. result", result[3].value, "210");
    a.checkEqual("12. result", result[3].unit, "kt");
    a.checkEqual("13. result", result[4].name, "\xE2\x96\xB6 Min. Cargo");
    a.checkEqual("14. result", result[4].value, "60");
    a.checkEqual("15. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:torper", a)
{
    // Given a torper hull with 300 kt...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMass(300);
    h->setMaxCargo(2000);
    h->setMaxFuel(150);
    h->setMaxBeams(7);
    h->setMaxLaunchers(5);
    game::test::initPList32Beams(env.shipList);
    game::test::initPList32Torpedoes(env.shipList);

    // ...and a ship with 510 kt...
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect a valid result: reasoning includes weapons
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Current mass: 510 kt");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "Hull");
    a.checkEqual("05. result", result[1].value, "300");
    a.checkEqual("06. result", result[1].unit, "kt");
    a.checkEqual("07. result", result[2].name, "Cargo+Fuel+Weapons");
    a.checkEqual("08. result", result[2].value, "210");
    a.checkEqual("09. result", result[2].unit, "kt");
    a.checkEqual("10. result", result[3].name, "\xE2\x96\xB6 Max. Cargo+Weapons");
    a.checkEqual("11. result", result[3].value, "210");
    a.checkEqual("12. result", result[3].unit, "kt");
    a.checkEqual("13. result", result[4].name, "\xE2\x96\xB6 Min. Cargo");
    a.checkEqual("14. result", result[4].value, "4");
    a.checkEqual("15. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipMassRanges:torper2", a)
{
    // Given a torper hull with 300 kt...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMass(300);
    h->setMaxCargo(2000);
    h->setMaxFuel(150);
    h->setMaxBeams(7);
    h->setMaxLaunchers(5);
    game::test::initPList32Beams(env.shipList);
    game::test::initPList32Torpedoes(env.shipList);

    // ...and a ship with 510 kt and known equipment...
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setHull(77);
    sh.setTorpedoType(3);
    sh.setNumLaunchers(5);
    sh.setBeamType(4);
    sh.setNumBeams(6);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect a valid result: reasoning includes weapons
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 7U);
    a.checkEqual("02. result", result[0].name, "Current mass: 510 kt");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "Hull");
    a.checkEqual("05. result", result[1].value, "300");
    a.checkEqual("06. result", result[1].unit, "kt");
    a.checkEqual("07. result", result[2].name, "Fusion Bomb launchers");
    a.checkEqual("08. result", result[2].value, "15");
    a.checkEqual("09. result", result[2].unit, "kt");
    a.checkEqual("10. result", result[3].name, "Phaser beams");
    a.checkEqual("11. result", result[3].value, "6");
    a.checkEqual("12. result", result[3].unit, "kt");
    a.checkEqual("13. result", result[4].name, "Cargo+Fuel");
    a.checkEqual("14. result", result[4].value, "189");
    a.checkEqual("15. result", result[4].unit, "kt");
    a.checkEqual("16. result", result[5].name, "\xE2\x96\xB6 Max. Cargo");
    a.checkEqual("17. result", result[5].value, "189");
    a.checkEqual("18. result", result[5].unit, "kt");
    a.checkEqual("19. result", result[6].name, "\xE2\x96\xB6 Min. Cargo");
    a.checkEqual("20. result", result[6].value, "39");
    a.checkEqual("21. result", result[6].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipLastKnownCargo:empty", a)
{
    // Given a ship with no information...
    Environment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect an empty result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 0U);
}

AFL_TEST("game.map.ShipInfo:packShipLastKnownCargo:carrier", a)
{
    // Given a carrier with some information...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setNumBays(1);

    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setCargo(game::Element::Tritanium, 20);
    sh.setAmmo(10);
    sh.setNumBays(1);
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect an appropriate result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Last known cargo");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "(age of this information is unknown)");
    a.checkEqual("05. result", result[2].name, "Tritanium");
    a.checkEqual("06. result", result[2].value, "20");
    a.checkEqual("07. result", result[2].unit, "kt");
    a.checkEqual("08. result", result[3].name, "Fighters");
    a.checkEqual("09. result", result[3].value, "10");
    a.checkEqual("10. result", result[3].unit, "");
    a.checkEqual("11. result", result[4].name, "\xE2\x96\xB6 Total");
    a.checkEqual("12. result", result[4].value, "30");
    a.checkEqual("13. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipLastKnownCargo:torper", a)
{
    // Given a torper with some information...
    Environment env;
    Hull* h = env.shipList.hulls().create(77);
    h->setMaxLaunchers(7);
    game::test::initPList32Torpedoes(env.shipList);

    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.setCargo(game::Element::Neutronium, 20);
    sh.setAmmo(10);
    sh.setNumLaunchers(1);
    sh.setTorpedoType(3);
    sh.setHull(77);
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect an appropriate result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    a.checkEqual("01. size", result.size(), 5U);
    a.checkEqual("02. result", result[0].name, "Last known cargo");
    a.checkEqual("03. result", result[0].isHeading, true);
    a.checkEqual("04. result", result[1].name, "(age of this information is unknown)");
    a.checkEqual("05. result", result[2].name, "Neutronium");
    a.checkEqual("06. result", result[2].value, "20");
    a.checkEqual("07. result", result[2].unit, "kt");
    a.checkEqual("08. result", result[3].name, "Fusion Bomb");
    a.checkEqual("09. result", result[3].value, "10");
    a.checkEqual("10. result", result[3].unit, "");
    a.checkEqual("11. result", result[4].name, "\xE2\x96\xB6 Total");
    a.checkEqual("12. result", result[4].value, "30");
    a.checkEqual("13. result", result[4].unit, "kt");
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:empty", a)
{
    // Given an unknown ship...
    MoveEnvironment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.internalCheck(game::PlayerSet_t(4), 15);

    // ...I expect no movement information
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);
    a.checkEqual("01. size", result.size(), 0U);
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:normal", a)
{
    // Given a known, played ship...
    MoveEnvironment env;
    Ship& sh = addShip(a, env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));

    // ...I regular movement information
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);

    a.checkEqual("01. size", result.size(), 1U);
    a.checkEqual("02. result", result[0].action, ShipMovementInfo::Movement);
    a.checkEqual("03. result", result[0].status, ShipMovementInfo::Success);
    a.checkEqual("04. result", result[0].from, Point(1000, 1100));
    a.checkEqual("05. result", result[0].to, Point(1200, 1500));

    // Test same thing using ==, !=
    a.check("11", result[0] == ShipMovementInfo(ShipMovementInfo::Movement, ShipMovementInfo::Success, 0, Point(1000, 1100), Point(1200, 1500)));
    a.check("12", result[0] != ShipMovementInfo());
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:tow", a)
{
    // Given a known, played ship...
    MoveEnvironment env;
    Ship& sh = addShip(a, env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));

    // ...that is towed by another ship...
    Ship& sh2 = addShip(a, env, 20, Point(1000, 1100));
    sh2.setWaypoint(Point(1300, 1200));
    sh2.setMission(game::spec::Mission::msn_Tow, 0, 10);

    // ...I regular movement information and tow information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);

    a.check("01", hasInfo(result, ShipMovementInfo(ShipMovementInfo::Movement, ShipMovementInfo::Success, 0,  Point(1000, 1100), Point(1200, 1500))));
    a.check("02", hasInfo(result, ShipMovementInfo(ShipMovementInfo::Tow,      ShipMovementInfo::Success, 20, Point(1000, 1100), Point(1300, 1200))));
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:chunnel", a)
{
    // Given a known, played, chunnelable ship...
    MoveEnvironment env;
    Ship& sh = addShip(a, env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1000, 1100));
    sh.setWarpFactor(0);
    sh.setFriendlyCode(String_t("123"));
    sh.setCargo(game::Element::Neutronium, 100);
    sh.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...that chunneling to another ship...
    Ship& sh2 = addShip(a, env, 123, Point(2000, 1100));
    sh2.setWaypoint(Point(2000, 1100));
    sh2.setWarpFactor(0);
    sh2.setCargo(game::Element::Neutronium, 100);
    sh2.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...I expect chunnel information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);

    a.check("01", hasInfo(result, ShipMovementInfo(ShipMovementInfo::Chunnel, ShipMovementInfo::Success, 123, Point(1000, 1100), Point(2000, 1100))));
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:chunnel:fail", a)
{
    // Given a known, played, chunnelable ship...
    MoveEnvironment env;
    Ship& sh = addShip(a, env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1000, 1100));
    sh.setWarpFactor(0);
    sh.setFriendlyCode(String_t("123"));
    sh.setCargo(game::Element::Neutronium, 100);
    sh.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...that chunneling to another ship, which has no fuel...
    Ship& sh2 = addShip(a, env, 123, Point(2000, 1100));
    sh2.setWaypoint(Point(2000, 1100));
    sh2.setWarpFactor(0);
    sh2.setCargo(game::Element::Neutronium, 0);
    sh2.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...I expect chunnel information with failure notice.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);

    a.check("01", hasInfo(result, ShipMovementInfo(ShipMovementInfo::Chunnel, ShipMovementInfo::MateFails, 123, Point(1000, 1100), Point(2000, 1100))));
}

AFL_TEST("game.map.ShipInfo:packShipMovementInfo:fleet", a)
{
    // Given a known, played ship that is member of a fleet...
    MoveEnvironment env;
    Ship& sh = addShip(a, env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));
    sh.setFleetNumber(42);

    // ...and a fleet leader at a different position...
    Ship& sh2 = addShip(a, env, 42, Point(1300, 1000));
    sh2.setWaypoint(Point(1400, 1200));
    sh2.setFleetNumber(42);

    // ...I regular movement information and fleet leader information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.teams, env.shipList, *env.root);

    a.check("01", hasInfo(result, ShipMovementInfo(ShipMovementInfo::Movement,    ShipMovementInfo::Success, 0,  Point(1000, 1100), Point(1200, 1500))));
    a.check("02", hasInfo(result, ShipMovementInfo(ShipMovementInfo::FleetLeader, ShipMovementInfo::Success, 42, Point(1000, 1100), Point(1300, 1000))));
}
