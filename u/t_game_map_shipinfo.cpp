/**
  *  \file u/t_game_map_shipinfo.cpp
  *  \brief Test for game::map::ShipInfo
  */

#include <algorithm>
#include "game/map/shipinfo.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

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
        game::spec::ShipList shipList;
        afl::base::Ref<game::Root> root;

        MoveEnvironment()
            : univ(), shipScores(), mapConfig(), shipList(), root(game::test::makeRoot(game::HostVersion()))
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

    Ship& addShip(MoveEnvironment& env, int id, Point pos)
    {
        Ship* sh = env.univ.ships().create(id);
        TS_ASSERT(sh != 0);

        game::map::ShipData sd;
        sd.owner = 4;
        sd.x = pos.getX();
        sd.y = pos.getY();
        sd.hullType = game::test::ANNIHILATION_HULL_ID;
        sh->addCurrentShipData(sd, game::PlayerSet_t(4));
        sh->internalCheck();
        sh->combinedCheck1(env.univ, game::PlayerSet_t(4), /* turn: */ 15);
        sh->setPlayability(Ship::Playable);
        return *sh;
    }

    bool hasInfo(const game::map::ShipMovementInfos_t& result, const ShipMovementInfo& ele)
    {
        return std::find(result.begin(), result.end(), ele) != result.end();
    }
}

/** Test packShipLocationInfo(). */
void
TestGameMapShipInfo::testPackShipLocationInfo()
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
    pl->internalCheck(mapConfig, tx, log);

    // Create a ship
    Ship sh(33);
    sh.setHull(HULL_NR);
    sh.setEngineType(9);
    sh.addShipXYData(PA, 3, 300, game::PlayerSet_t(10));
    sh.setWaypoint(Point(1000, 900));
    sh.setWarpFactor(3);
    addShipTrack(sh, TURN_NR-1, PB);
    addShipTrack(sh, TURN_NR-2, PC);
    sh.internalCheck();
    sh.combinedCheck1(univ, game::PlayerSet_t(10), TURN_NR);

    // Do it
    game::map::ShipLocationInfos_t result;
    packShipLocationInfo(result, sh, univ, TURN_NR, mapConfig, config, host, sl, tx);

    // Verify
    TS_ASSERT(result.size() >= 3);
    TS_ASSERT(result.size() <= TURN_NR);

    TS_ASSERT_EQUALS(result[0].turnNumber, TURN_NR);
    TS_ASSERT_EQUALS(result[0].position.isValid(), true);
    TS_ASSERT_EQUALS(result[0].position.get()->getX(), 1000);
    TS_ASSERT_EQUALS(result[0].position.get()->getY(), 1000);
    TS_ASSERT_EQUALS(result[0].positionName, "(1000,1000)");
    TS_ASSERT_EQUALS(result[0].mass.orElse(-1), 300);            // from shipxy, because it is a scanned ship
    TS_ASSERT_EQUALS(result[0].heading.orElse(-1), 180);         // actual angle, not from history
    TS_ASSERT_EQUALS(result[0].warpFactor.orElse(-1), 3);
    TS_ASSERT_EQUALS(int(result[0].distanceMoved.orElse(-1)), 141);

    TS_ASSERT_EQUALS(result[1].turnNumber, TURN_NR-1);
    TS_ASSERT_EQUALS(result[1].position.isValid(), true);
    TS_ASSERT_EQUALS(result[1].position.get()->getX(), 1100);
    TS_ASSERT_EQUALS(result[1].position.get()->getY(), 1100);
    TS_ASSERT_EQUALS(result[1].positionName, "Orbit of Pluto (#99)");
    TS_ASSERT_EQUALS(result[1].mass.orElse(-1), 100);            // from history
    TS_ASSERT_EQUALS(result[1].heading.isValid(), false);        // from history
    TS_ASSERT_EQUALS(result[1].warpFactor.isValid(), false);
    TS_ASSERT_EQUALS(int(result[1].distanceMoved.orElse(-1)), 50);

    TS_ASSERT_EQUALS(result[2].turnNumber, TURN_NR-2);
    TS_ASSERT_EQUALS(result[2].position.isValid(), true);
    TS_ASSERT_EQUALS(result[2].position.get()->getX(), 1150);
    TS_ASSERT_EQUALS(result[2].position.get()->getY(), 1100);
    TS_ASSERT_EQUALS(result[2].positionName, "(1150,1100)");
    TS_ASSERT_EQUALS(result[2].mass.orElse(-1), 100);            // from history
    TS_ASSERT_EQUALS(result[2].heading.isValid(), false);        // from history
    TS_ASSERT_EQUALS(result[2].warpFactor.isValid(), false);
    TS_ASSERT_EQUALS(result[2].distanceMoved.isValid(), false);

    if (result.size() > 3) {
        TS_ASSERT_EQUALS(result[3].turnNumber, TURN_NR-3);
        TS_ASSERT_EQUALS(result[3].position.isValid(), false);
    }
}

void
TestGameMapShipInfo::testPackShipMassRanges()
{
    // Given a ship with unknown hull...
    Environment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 400, game::PlayerSet_t(4));
    sh.internalCheck();

    // ...I expect no result.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);
    TS_ASSERT_EQUALS(result.size(), 0U);
}

void
TestGameMapShipInfo::testPackShipMassRanges2()
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
    sh.internalCheck();

    // ...I expect a valid result: scanned mass indicates neither tank, nor cargo hold are full, but they might be empty.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Current mass: 400 kt");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "Hull");
    TS_ASSERT_EQUALS(result[1].value, "300");
    TS_ASSERT_EQUALS(result[1].unit, "kt");
    TS_ASSERT_EQUALS(result[2].name, "Cargo+Fuel");
    TS_ASSERT_EQUALS(result[2].value, "100");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "\xE2\x96\xB6 Max. Fuel");
    TS_ASSERT_EQUALS(result[3].value, "100");
    TS_ASSERT_EQUALS(result[3].unit, "kt");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Max. Cargo");
    TS_ASSERT_EQUALS(result[4].value, "100");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipMassRanges3()
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
    sh.internalCheck();

    // ...I expect a valid result: scanned mass indicates neither tank, nor cargo hold are empty, but they might be full.
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Current mass: 510 kt");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "Hull");
    TS_ASSERT_EQUALS(result[1].value, "300");
    TS_ASSERT_EQUALS(result[1].unit, "kt");
    TS_ASSERT_EQUALS(result[2].name, "Cargo+Fuel");
    TS_ASSERT_EQUALS(result[2].value, "210");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "\xE2\x96\xB6 Min. Fuel");
    TS_ASSERT_EQUALS(result[3].value, "10");
    TS_ASSERT_EQUALS(result[3].unit, "kt");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Min. Cargo");
    TS_ASSERT_EQUALS(result[4].value, "60");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipMassRanges4()
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
    sh.internalCheck();

    // ...I expect a valid result: scanned mass indicates fuel tank might be full or empty, cargo follows from that
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Current mass: 510 kt");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "Hull");
    TS_ASSERT_EQUALS(result[1].value, "300");
    TS_ASSERT_EQUALS(result[1].unit, "kt");
    TS_ASSERT_EQUALS(result[2].name, "Cargo+Fuel");
    TS_ASSERT_EQUALS(result[2].value, "210");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "\xE2\x96\xB6 Max. Cargo");
    TS_ASSERT_EQUALS(result[3].value, "210");
    TS_ASSERT_EQUALS(result[3].unit, "kt");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Min. Cargo");
    TS_ASSERT_EQUALS(result[4].value, "60");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipMassRanges5()
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
    sh.internalCheck();

    // ...I expect a valid result: reasoning includes weapons
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Current mass: 510 kt");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "Hull");
    TS_ASSERT_EQUALS(result[1].value, "300");
    TS_ASSERT_EQUALS(result[1].unit, "kt");
    TS_ASSERT_EQUALS(result[2].name, "Cargo+Fuel+Weapons");
    TS_ASSERT_EQUALS(result[2].value, "210");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "\xE2\x96\xB6 Max. Cargo+Weapons");
    TS_ASSERT_EQUALS(result[3].value, "210");
    TS_ASSERT_EQUALS(result[3].unit, "kt");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Min. Cargo");
    TS_ASSERT_EQUALS(result[4].value, "4");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipMassRanges6()
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
    sh.internalCheck();

    // ...I expect a valid result: reasoning includes weapons
    game::map::ShipCargoInfos_t result;
    packShipMassRanges(result, sh, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 7U);
    TS_ASSERT_EQUALS(result[0].name, "Current mass: 510 kt");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "Hull");
    TS_ASSERT_EQUALS(result[1].value, "300");
    TS_ASSERT_EQUALS(result[1].unit, "kt");
    TS_ASSERT_EQUALS(result[2].name, "Fusion Bomb launchers");
    TS_ASSERT_EQUALS(result[2].value, "15");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "Phaser beams");
    TS_ASSERT_EQUALS(result[3].value, "6");
    TS_ASSERT_EQUALS(result[3].unit, "kt");
    TS_ASSERT_EQUALS(result[4].name, "Cargo+Fuel");
    TS_ASSERT_EQUALS(result[4].value, "189");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
    TS_ASSERT_EQUALS(result[5].name, "\xE2\x96\xB6 Max. Cargo");
    TS_ASSERT_EQUALS(result[5].value, "189");
    TS_ASSERT_EQUALS(result[5].unit, "kt");
    TS_ASSERT_EQUALS(result[6].name, "\xE2\x96\xB6 Min. Cargo");
    TS_ASSERT_EQUALS(result[6].value, "39");
    TS_ASSERT_EQUALS(result[6].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipLastKnownCargo()
{
    // Given a ship with no information...
    Environment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.internalCheck();

    // ...I expect an empty result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 0U);
}

void
TestGameMapShipInfo::testPackShipLastKnownCargo2()
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
    sh.internalCheck();

    // ...I expect an appropriate result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Last known cargo");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "(age of this information is unknown)");
    TS_ASSERT_EQUALS(result[2].name, "Tritanium");
    TS_ASSERT_EQUALS(result[2].value, "20");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "Fighters");
    TS_ASSERT_EQUALS(result[3].value, "10");
    TS_ASSERT_EQUALS(result[3].unit, "");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Total");
    TS_ASSERT_EQUALS(result[4].value, "30");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipLastKnownCargo3()
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
    sh.internalCheck();

    // ...I expect an appropriate result.
    game::map::ShipCargoInfos_t result;
    packShipLastKnownCargo(result, sh, 15, env.fmt, env.shipList, env.tx);

    TS_ASSERT_EQUALS(result.size(), 5U);
    TS_ASSERT_EQUALS(result[0].name, "Last known cargo");
    TS_ASSERT_EQUALS(result[0].isHeading, true);
    TS_ASSERT_EQUALS(result[1].name, "(age of this information is unknown)");
    TS_ASSERT_EQUALS(result[2].name, "Neutronium");
    TS_ASSERT_EQUALS(result[2].value, "20");
    TS_ASSERT_EQUALS(result[2].unit, "kt");
    TS_ASSERT_EQUALS(result[3].name, "Fusion Bomb");
    TS_ASSERT_EQUALS(result[3].value, "10");
    TS_ASSERT_EQUALS(result[3].unit, "");
    TS_ASSERT_EQUALS(result[4].name, "\xE2\x96\xB6 Total");
    TS_ASSERT_EQUALS(result[4].value, "30");
    TS_ASSERT_EQUALS(result[4].unit, "kt");
}

void
TestGameMapShipInfo::testPackShipMovementInfo()
{
    // Given an unknown ship...
    MoveEnvironment env;
    Ship sh(10);
    sh.addShipXYData(Point(1000, 1000), 3, 510, game::PlayerSet_t(4));
    sh.internalCheck();

    // ...I expect no movement information
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);
    TS_ASSERT_EQUALS(result.size(), 0U);
}

void
TestGameMapShipInfo::testPackShipMovementInfo2()
{
    // Given a known, played ship...
    MoveEnvironment env;
    Ship& sh = addShip(env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));

    // ...I regular movement information
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);

    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT_EQUALS(result[0].action, ShipMovementInfo::Movement);
    TS_ASSERT_EQUALS(result[0].status, ShipMovementInfo::Success);
    TS_ASSERT_EQUALS(result[0].from, Point(1000, 1100));
    TS_ASSERT_EQUALS(result[0].to, Point(1200, 1500));

    // Test same thing using ==, !=
    TS_ASSERT(result[0] == ShipMovementInfo(ShipMovementInfo::Movement, ShipMovementInfo::Success, 0, Point(1000, 1100), Point(1200, 1500)));
    TS_ASSERT(result[0] != ShipMovementInfo());
}

void
TestGameMapShipInfo::testPackShipMovementInfoTow()
{
    // Given a known, played ship...
    MoveEnvironment env;
    Ship& sh = addShip(env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));

    // ...that is towed by another ship...
    Ship& sh2 = addShip(env, 20, Point(1000, 1100));
    sh2.setWaypoint(Point(1300, 1200));
    sh2.setMission(game::spec::Mission::msn_Tow, 0, 10);

    // ...I regular movement information and tow information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);

    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::Movement, ShipMovementInfo::Success, 0,  Point(1000, 1100), Point(1200, 1500))));
    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::Tow,      ShipMovementInfo::Success, 20, Point(1000, 1100), Point(1300, 1200))));
}

void
TestGameMapShipInfo::testPackShipMovementInfoChunnel()
{
    // Given a known, played, chunnelable ship...
    MoveEnvironment env;
    Ship& sh = addShip(env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1000, 1100));
    sh.setWarpFactor(0);
    sh.setFriendlyCode(String_t("123"));
    sh.setCargo(game::Element::Neutronium, 100);
    sh.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...that chunneling to another ship...
    Ship& sh2 = addShip(env, 123, Point(2000, 1100));
    sh2.setWaypoint(Point(2000, 1100));
    sh2.setWarpFactor(0);
    sh2.setCargo(game::Element::Neutronium, 100);
    sh2.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...I expect chunnel information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);

    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::Chunnel, ShipMovementInfo::Success, 123, Point(1000, 1100), Point(2000, 1100))));
}

void
TestGameMapShipInfo::testPackShipMovementInfoChunnelFail()
{
    // Given a known, played, chunnelable ship...
    MoveEnvironment env;
    Ship& sh = addShip(env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1000, 1100));
    sh.setWarpFactor(0);
    sh.setFriendlyCode(String_t("123"));
    sh.setCargo(game::Element::Neutronium, 100);
    sh.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...that chunneling to another ship, which has no fuel...
    Ship& sh2 = addShip(env, 123, Point(2000, 1100));
    sh2.setWaypoint(Point(2000, 1100));
    sh2.setWarpFactor(0);
    sh2.setCargo(game::Element::Neutronium, 0);
    sh2.addShipSpecialFunction(env.shipList.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::FirecloudChunnel));

    // ...I expect chunnel information with failure notice.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);

    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::Chunnel, ShipMovementInfo::MateFails, 123, Point(1000, 1100), Point(2000, 1100))));
}

void
TestGameMapShipInfo::testPackShipMovementInfoFleet()
{
    // Given a known, played ship that is member of a fleet...
    MoveEnvironment env;
    Ship& sh = addShip(env, 10, Point(1000, 1100));
    sh.setWaypoint(Point(1200, 1500));
    sh.setFleetNumber(42);

    // ...and a fleet leader at a different position...
    Ship& sh2 = addShip(env, 42, Point(1300, 1000));
    sh2.setWaypoint(Point(1400, 1200));
    sh2.setFleetNumber(42);

    // ...I regular movement information and fleet leader information.
    game::map::ShipMovementInfos_t result;
    packShipMovementInfo(result, sh, env.univ, env.shipScores, env.mapConfig, env.shipList, *env.root);

    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::Movement,    ShipMovementInfo::Success, 0,  Point(1000, 1100), Point(1200, 1500))));
    TS_ASSERT(hasInfo(result, ShipMovementInfo(ShipMovementInfo::FleetLeader, ShipMovementInfo::Success, 42, Point(1000, 1100), Point(1300, 1000))));
}

