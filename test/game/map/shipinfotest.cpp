/**
  *  \file test/game/map/shipinfotest.cpp
  *  \brief Test for game::map::ShipInfo
  */

#include "game/map/shipinfo.hpp"

#include <algorithm>
#include "afl/base/countof.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

using afl::base::Ref;
using game::HostVersion;
using game::UnitScoreDefinitionList;
using game::config::HostConfiguration;
using game::map::Point;
using game::map::Ship;
using game::map::ShipMovementInfo;
using game::spec::Hull;
using game::spec::ShipList;

namespace {
    struct Environment {
        util::NumberFormatter fmt;
        ShipList shipList;
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
        ShipList shipList;
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
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::HostVersion host;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Ship list
    ShipList sl;
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
    packShipLocationInfo(result, sh, univ, TURN_NR, mapConfig, *config, host, sl, tx);

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

AFL_TEST("game.map.ShipInfo:getShipTrainingExperience:rebel-small", a)
{
    // Configuration from North Star series
    // c2hosttest test case ship/06_training/rebel-small
    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::EPTrainingScale].set("45,50,40,55,45,50,48,50,55,70,50");

    static const int16_t EXPECT[] = {
         11,    23,    35,    46,    58,    70,    81,    93,   105,   116,   128,   140,   151,   163,   175,   186,   198,   210,   221,   233,
        245,   256,   268,   280,   291,   324,   338,   348,   357,   365,   372,   378,   385,   390,   396,   401,   405,   410,   415,   419,
        423,   427,   431,   435,   439,   442,   446,   449,   453,   456,   459,   463,   466,   469,   472,   475,   478,   481,   484,   486,
        489,   492,   495,   497,   500,   502,   505,   508,   510,   513,   515,   517,   520,   522,   525,   527,   529,   531,   534,   536,
        538,   540,   542,   545,   547,   549,   551,   553,   555,   557,   559,   561,   563,   565,   567,   569,   571,   573,   575,   577,
        579,   581,   583,   584,   586,   588,   590,   592,   594,   595,   597,   599,   601,   602,   604,   606,   608,   609,   611,   613,
        614,   616,   618,   619,   621,   623,   624,   626,   628,   629,   631,   633,   634,   636,   637,   639,   640,   642,   643,   645,
        647,   648,   650,   651,   653,   654,   656,   657,   659,   660,   662,   663,   665,   666,   667,   669,   670,   672,   673,   675,
        676,   677,   679,   680,   682,   683,   684,   686,   687,   689,   690,   691,   693,   694,   695,   697,   698,   699,   701,   702,
        703,   705,   706,   707,   709,   710,   711,   712,   714,   715,   716,   718,   719,   720,   721,   723,   724,   725,   726,   728,
        729,   730,   731,   733,   734,   735,   736,   738,   739,   740,   741,   742,   744,   745,   746,   747,   748,   750,   751,   752,
        753,   754,   755,   757,   758,   759,   760,   761,   762,   764,   765,   766,   767,   768,   769,   770,   772,   773,   774,   775,
        776,   777,   778,   779,   781,   782,   783,   784,   785,   786,   787,   788,   789,   791,   792,   793,   794,   795,   796,   797,
        798,   799,   800,   801,   802,   803,   805,   806,   807,   808,   809,   810,   811,   812,   813,   814,   815,   816,   817,   818,
        819,   820,   821,   822,   823,   824,   825,   826,   827,   828,   829,   830,   831,   832,   833,   834,   835,   836,   837,   838,
        839,   840,   841,   842,   843,   844,   845,   846,   847,   848,   849,   850,   851,   852,   853,   854,   855,   856,   857,   858,
        859,   860,   861,   862,   863,   864,   865,   866,   867,   867,   868,   869,   870,   871,   872,   873,   874,   875,   876,   877,
        878,   879,   880,   881,   881,   882,   883,   884,   885,   886,   887,   888,   889,   890,   891,   892,   892,   893,   894,   895,
        896,   897,   898,   899,   900,   901,   901,   902,   903,   904,   905,   906,   907,   908,   909,   909,   910,   911,   912,   913,
        914,   915,   916,   916,   917,   918,   919,   920,   921,   922,   922,   923,   924,   925,   926,   927,   928,   928,   929,   930,
        931,   932,   933,   934,   934,   935,   936,   937,   938,   939,   939,   940,   941,   942,   943,   944,   945,   945,   946,   947,
        948,   949,   949,   950,   951,   952,   953,   954,   954,   955,   956,   957,   958,   959,   959,   960,   961,   962,   963,   963,
        964,   965,   966,   967,   967,   968,   969,   970,   971,   971,   972,   973,   974,   975,   975,   976,   977,   978,   979,   979,
        980,   981,   982,   983,   983,   984,   985,   986,   986,   987,   988,   989,   990,   990,   991,   992,   993,   993,   994,   995,
        996,   997,   997,   998,   999,  1000,  1000,  1001,  1002,  1003,  1004,  1004,  1005,  1006,  1007,  1007,  1008,  1009,  1010,  1010
    };

    for (int i = 0; i < int(countof(EXPECT)); ++i) {
        a.checkEqual("", game::map::getShipTrainingExperience(10,  i+1, false, 25, *config), EXPECT[i]);
    }
}

AFL_TEST("game.map.ShipInfo:getShipTrainingExperience:rebel-big", a)
{
    // Configuration from North Star series
    // c2hosttest test case ship/06_training/rebel-big
    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::EPTrainingScale].set("45,50,40,55,45,50,48,50,55,70,50");

    static const int16_t EXPECT[] = {
          4,     8,    13,    17,    21,    26,    30,    35,    39,    43,    48,    52,    56,    61,    65,    70,    74,    78,    83,    87,
         92,    96,   100,   105,   109,   122,   127,   131,   134,   137,   139,   142,   144,   146,   148,   150,   152,   154,   156,   157,
        159,   160,   162,   163,   165,   166,   167,   169,   170,   171,   172,   174,   175,   176,   177,   178,   179,   180,   181,   182,
        184,   185,   186,   187,   188,   189,   189,   190,   191,   192,   193,   194,   195,   196,   197,   198,   199,   199,   200,   201,
        202,   203,   204,   204,   205,   206,   207,   208,   208,   209,   210,   211,   211,   212,   213,   214,   214,   215,   216,   216,
        217,   218,   219,   219,   220,   221,   221,   222,   223,   223,   224,   225,   225,   226,   227,   227,   228,   229,   229,   230,
        231,   231,   232,   232,   233,   234,   234,   235,   236,   236,   237,   237,   238,   239,   239,   240,   240,   241,   242,   242,
        243,   243,   244,   244,   245,   246,   246,   247,   247,   248,   248,   249,   249,   250,   250,   251,   252,   252,   253,   253,
        254,   254,   255,   255,   256,   256,   257,   257,   258,   258,   259,   259,   260,   260,   261,   261,   262,   262,   263,   263,
        264,   264,   265,   265,   266,   266,   267,   267,   268,   268,   269,   269,   270,   270,   271,   271,   272,   272,   273,   273,
        274,   274,   275,   275,   275,   276,   276,   277,   277,   278,   278,   279,   279,   280,   280,   280,   281,   281,   282,   282,
        283,   283,   284,   284,   284,   285,   285,   286,   286,   287,   287,   288,   288,   288,   289,   289,   290,   290,   291,   291,
        291,   292,   292,   293,   293,   293,   294,   294,   295,   295,   296,   296,   296,   297,   297,   298,   298,   298,   299,   299,
        300,   300,   300,   301,   301,   302,   302,   302,   303,   303,   304,   304,   304,   305,   305,   306,   306,   306,   307,   307,
        308,   308,   308,   309,   309,   309,   310,   310,   311,   311,   311,   312,   312,   312,   313,   313,   314,   314,   314,   315,
        315,   315,   316,   316,   317,   317,   317,   318,   318,   318,   319,   319,   320,   320,   320,   321,   321,   321,   322,   322,
        322,   323,   323,   324,   324,   324,   325,   325,   325,   326,   326,   326,   327,   327,   327,   328,   328,   328,   329,   329,
        330,   330,   330,   331,   331,   331,   332,   332,   332,   333,   333,   333,   334,   334,   334,   335,   335,   335,   336,   336,
        336,   337,   337,   337,   338,   338,   338,   339,   339,   339,   340,   340,   340,   341,   341,   341,   342,   342,   342,   343,
        343,   343,   344,   344,   344,   345,   345,   345,   346,   346,   346,   347,   347,   347,   348,   348,   348,   349,   349,   349,
        350,   350,   350,   351,   351,   351,   351,   352,   352,   352,   353,   353,   353,   354,   354,   354,   355,   355,   355,   356,
        356,   356,   356,   357,   357,   357,   358,   358,   358,   359,   359,   359,   360,   360,   360,   360,   361,   361,   361,   362,
        362,   362,   363,   363,   363,   364,   364,   364,   364,   365,   365,   365,   366,   366,   366,   367,   367,   367,   367,   368,
        368,   368,   369,   369,   369,   370,   370,   370,   370,   371,   371,   371,   372,   372,   372,   372,   373,   373,   373,   374,
        374,   374,   374,   375,   375,   375,   376,   376,   376,   377,   377,   377,   377,   378,   378,   378,   379,   379,   379,   379
    };

    for (int i = 0; i < int(countof(EXPECT)); ++i) {
        a.checkEqual("", game::map::getShipTrainingExperience(10,  i+1, false, 224, *config), EXPECT[i]);
    }
}

AFL_TEST("game.map.ShipInfo:getShipTrainingExperience:rebel-academy", a)
{
    // Configuration from North Star series
    // c2hosttest test case ship/06_training/rebel-academy
    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::EPTrainingScale].set("45,50,40,55,45,50,48,50,55,70,50");

    static const int16_t EXPECT[] = {
          46,    93,   140,   186,   233,   280,   326,   373,   420,   466,   513,   560,   606,   653,   700,   746,   793,   840,   886,   933,
         980,  1026,  1073,  1120,  1166,  1298,  1353,  1395,  1430,  1461,  1489,  1515,  1540,  1562,  1584,  1604,  1623,  1642,  1660,  1677,
        1694,  1710,  1726,  1742,  1756,  1771,  1785,  1799,  1813,  1826,  1839,  1852,  1865,  1877,  1889,  1901,  1913,  1924,  1936,  1947,
        1958,  1969,  1980,  1990,  2001,  2011,  2022,  2032,  2042,  2052,  2061,  2071,  2081,  2090,  2100,  2109,  2118,  2127,  2136,  2145,
        2154,  2163,  2171,  2180,  2189,  2197,  2205,  2214,  2222,  2230,  2238,  2247,  2255,  2263,  2271,  2278,  2286,  2294,  2302,  2309,
        2317,  2324,  2332,  2339,  2347,  2354,  2361,  2369,  2376,  2383,  2390,  2397,  2404,  2411,  2418,  2425,  2432,  2439,  2446,  2453,
        2459,  2466,  2473,  2479,  2486,  2493,  2499,  2506,  2512,  2519,  2525,  2532,  2538,  2544,  2551,  2557,  2563,  2569,  2575,  2582,
        2588,  2594,  2600,  2606,  2612,  2618,  2624,  2630,  2636,  2642,  2648,  2654,  2660,  2665,  2671,  2677,  2683,  2688,  2694,  2700,
        2705,  2711,  2717,  2722,  2728,  2733,  2739,  2745,  2750,  2756,  2761,  2766,  2772,  2777,  2783,  2788,  2793,  2799,  2804,  2809,
        2815,  2820,  2825,  2831,  2836,  2841,  2846,  2851,  2857,  2862,  2867,  2872,  2877,  2882,  2887,  2892,  2897,  2902,  2907,  2912,
        2917,  2922,  2927,  2932,  2937,  2942,  2947,  2952,  2957,  2961,  2966,  2971,  2976,  2981,  2986,  2990,  2995,  3000,  3005,  3009,
        3014,  3019,  3023,  3028,  3033,  3037,  3042,  3047,  3051,  3056,  3061,  3065,  3070,  3074,  3079,  3083,  3088,  3093,  3097,  3102,
        3106,  3111,  3115,  3119,  3124,  3128,  3133,  3137,  3142,  3146,  3150,  3155,  3159,  3164,  3168,  3172,  3177,  3181,  3185,  3190,
        3194,  3198,  3202,  3207,  3211,  3215,  3220,  3224,  3228,  3232,  3236,  3241,  3245,  3249,  3253,  3257,  3261,  3266,  3270,  3274,
        3278,  3282,  3286,  3290,  3294,  3299,  3303,  3307,  3311,  3315,  3319,  3323,  3327,  3331,  3335,  3339,  3343,  3347,  3351,  3355,
        3359,  3363,  3367,  3371,  3375,  3379,  3383,  3387,  3391,  3394,  3398,  3402,  3406,  3410,  3414,  3418,  3422,  3426,  3429,  3433,
        3437,  3441,  3445,  3449,  3452,  3456,  3460,  3464,  3468,  3471,  3475,  3479,  3483,  3486,  3490,  3494,  3498,  3501,  3505,  3509,
        3513,  3516,  3520,  3524,  3527,  3531,  3535,  3538,  3542,  3546,  3549,  3553,  3557,  3560,  3564,  3568,  3571,  3575,  3578,  3582,
        3586,  3589,  3593,  3596,  3600,  3604,  3607,  3611,  3614,  3618,  3621,  3625,  3628,  3632,  3636,  3639,  3643,  3646,  3650,  3653,
        3657,  3660,  3664,  3667,  3671,  3674,  3678,  3681,  3684,  3688,  3691,  3695,  3698,  3702,  3705,  3709,  3712,  3715,  3719,  3722,
        3726,  3729,  3732,  3736,  3739,  3743,  3746,  3749,  3753,  3756,  3759,  3763,  3766,  3769,  3773,  3776,  3780,  3783,  3786,  3789,
        3793,  3796,  3799,  3803,  3806,  3809,  3813,  3816,  3819,  3822,  3826,  3829,  3832,  3836,  3839,  3842,  3845,  3849,  3852,  3855,
        3858,  3862,  3865,  3868,  3871,  3874,  3878,  3881,  3884,  3887,  3890,  3894,  3897,  3900,  3903,  3906,  3910,  3913,  3916,  3919,
        3922,  3925,  3929,  3932,  3935,  3938,  3941,  3944,  3947,  3951,  3954,  3957,  3960,  3963,  3966,  3969,  3972,  3975,  3979,  3982,
        3985,  3988,  3991,  3994,  3997,  4000,  4003,  4006,  4009,  4012,  4016,  4019,  4022,  4025,  4028,  4031,  4034,  4037,  4040,  4043
    };

    for (int i = 0; i < int(countof(EXPECT)); ++i) {
        a.checkEqual("", game::map::getShipTrainingExperience(10,  i+1, true, 25, *config), EXPECT[i]);
    }
}

AFL_TEST("game.map.ShipInfo:getShipTrainingExperience:bird-small", a)
{
    // Configuration from North Star series
    // c2hosttest test case ship/06_training/bird-small
    Ref<HostConfiguration> config = HostConfiguration::create();
    (*config)[HostConfiguration::EPTrainingScale].set("45,50,40,55,45,50,48,50,55,70,50");

    static const int16_t EXPECT[] = {
          6,    13,    20,    26,    33,    40,    46,    53,    60,    66,    73,    80,    86,    93,   100,   106,   113,   120,   126,   133,
        140,   146,   153,   160,   166,   185,   193,   199,   204,   208,   212,   216,   220,   223,   226,   229,   231,   234,   237,   239,
        242,   244,   246,   248,   250,   253,   255,   257,   259,   260,   262,   264,   266,   268,   269,   271,   273,   274,   276,   278,
        279,   281,   282,   284,   285,   287,   288,   290,   291,   293,   294,   295,   297,   298,   300,   301,   302,   303,   305,   306,
        307,   309,   310,   311,   312,   313,   315,   316,   317,   318,   319,   321,   322,   323,   324,   325,   326,   327,   328,   329,
        331,   332,   333,   334,   335,   336,   337,   338,   339,   340,   341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
        351,   352,   353,   354,   355,   356,   357,   358,   358,   359,   360,   361,   362,   363,   364,   365,   366,   367,   367,   368,
        369,   370,   371,   372,   373,   374,   374,   375,   376,   377,   378,   379,   380,   380,   381,   382,   383,   384,   384,   385,
        386,   387,   388,   388,   389,   390,   391,   392,   392,   393,   394,   395,   396,   396,   397,   398,   399,   399,   400,   401,
        402,   402,   403,   404,   405,   405,   406,   407,   408,   408,   409,   410,   411,   411,   412,   413,   413,   414,   415,   416,
        416,   417,   418,   418,   419,   420,   421,   421,   422,   423,   423,   424,   425,   425,   426,   427,   427,   428,   429,   429,
        430,   431,   431,   432,   433,   433,   434,   435,   435,   436,   437,   437,   438,   439,   439,   440,   441,   441,   442,   443,
        443,   444,   445,   445,   446,   446,   447,   448,   448,   449,   450,   450,   451,   452,   452,   453,   453,   454,   455,   455,
        456,   456,   457,   458,   458,   459,   460,   460,   461,   461,   462,   463,   463,   464,   464,   465,   465,   466,   467,   467,
        468,   468,   469,   470,   470,   471,   471,   472,   473,   473,   474,   474,   475,   475,   476,   477,   477,   478,   478,   479,
        479,   480,   481,   481,   482,   482,   483,   483,   484,   484,   485,   486,   486,   487,   487,   488,   488,   489,   489,   490,
        491,   491,   492,   492,   493,   493,   494,   494,   495,   495,   496,   497,   497,   498,   498,   499,   499,   500,   500,   501,
        501,   502,   502,   503,   503,   504,   505,   505,   506,   506,   507,   507,   508,   508,   509,   509,   510,   510,   511,   511,
        512,   512,   513,   513,   514,   514,   515,   515,   516,   516,   517,   517,   518,   518,   519,   519,   520,   520,   521,   521,
        522,   522,   523,   523,   524,   524,   525,   525,   526,   526,   527,   527,   528,   528,   529,   529,   530,   530,   531,   531,
        532,   532,   533,   533,   534,   534,   535,   535,   536,   536,   537,   537,   538,   538,   539,   539,   540,   540,   540,   541,
        541,   542,   542,   543,   543,   544,   544,   545,   545,   546,   546,   547,   547,   548,   548,   548,   549,   549,   550,   550,
        551,   551,   552,   552,   553,   553,   554,   554,   554,   555,   555,   556,   556,   557,   557,   558,   558,   559,   559,   559,
        560,   560,   561,   561,   562,   562,   563,   563,   563,   564,   564,   565,   565,   566,   566,   567,   567,   567,   568,   568,
        569,   569,   570,   570,   571,   571,   571,   572,   572,   573,   573,   574,   574,   575,   575,   575,   576,   576,   577,   577
    };

    for (int i = 0; i < int(countof(EXPECT)); ++i) {
        a.checkEqual("", game::map::getShipTrainingExperience(3,  i+1, false, 25, *config), EXPECT[i]);
    }
}

AFL_TEST("game.map.ShipInfo:packShipExperienceInfo:empty", a)
{
    Ship sh(42);
    UnitScoreDefinitionList scoreDefs;
    Ref<HostConfiguration> config = HostConfiguration::create();
    HostVersion host(HostVersion::PHost, MKVERSION(4,0,0));
    ShipList list;

    (*config)[HostConfiguration::NumExperienceLevels].set(4);

    sh.setOwner(3);
    sh.setHull(77);
    list.hulls().create(77)->setMaxCrew(42);

    a.checkEqual("01. EPShipAging", (*config)[HostConfiguration::EPShipAging](), 15);

    game::map::ShipExperienceInfo exp = game::map::packShipExperienceInfo(sh, scoreDefs, *config, host, list);
    a.check     ("11. level",       !exp.level.isValid());
    a.check     ("12. points",      !exp.points.isValid());
    a.checkEqual("13. pointGrowth",  exp.pointGrowth.orElse(-1), 15);
}

AFL_TEST("game.map.ShipInfo:packShipExperienceInfo:normal", a)
{
    Ship sh(42);
    UnitScoreDefinitionList scoreDefs;
    Ref<HostConfiguration> config = HostConfiguration::create();
    HostVersion host(HostVersion::PHost, MKVERSION(4,0,0));
    ShipList list;

    (*config)[HostConfiguration::NumExperienceLevels].set(4);
    (*config)[HostConfiguration::EPTrainingScale].set("45,50,40,55,45,50,48,50,55,70,50");

    // Definition of points
    UnitScoreDefinitionList::Definition pdef;
    pdef.name = "Points";
    pdef.id = game::ScoreId_ExpPoints;
    pdef.limit = 22222;
    UnitScoreDefinitionList::Index_t pindex = scoreDefs.add(pdef);

    // Definition of levels
    UnitScoreDefinitionList::Definition ldef;
    ldef.name = "Level";
    ldef.id = game::ScoreId_ExpLevel;
    ldef.limit = 4;
    UnitScoreDefinitionList::Index_t lindex = scoreDefs.add(ldef);

    sh.setOwner(3);
    sh.setHull(77);
    sh.setMission(38, 100, 0);
    sh.unitScores().set(pindex, 800, 10);
    sh.unitScores().set(lindex, 1, 10);
    list.hulls().create(77)->setMaxCrew(25);

    a.checkEqual("01. EPShipAging", (*config)[HostConfiguration::EPShipAging](), 15);

    game::map::ShipExperienceInfo exp = game::map::packShipExperienceInfo(sh, scoreDefs, *config, host, list);
    a.checkEqual("11. level",        exp.level.orElse(-1), 1);
    a.checkEqual("12. points",       exp.points.orElse(-1), 800);
    a.checkEqual("13. pointGrowth",  exp.pointGrowth.orElse(-1), 15 + 329);
}

AFL_TEST("game.map.ShipInfo:getNumTurnsUntil:empty", a)
{
    game::map::ShipExperienceInfo exp;
    a.checkEqual("01", getNumTurnsUntil(1000, exp), 0);
}

AFL_TEST("game.map.ShipInfo:getNumTurnsUntil:full", a)
{
    game::map::ShipExperienceInfo exp;
    exp.points = 500;
    exp.pointGrowth = 100;

    a.checkEqual("01", getNumTurnsUntil(100, exp), 0);
    a.checkEqual("02", getNumTurnsUntil(500, exp), 0);
    a.checkEqual("03", getNumTurnsUntil(501, exp), 1);
    a.checkEqual("04", getNumTurnsUntil(1000, exp), 5);
    a.checkEqual("05", getNumTurnsUntil(1001, exp), 6);
}
