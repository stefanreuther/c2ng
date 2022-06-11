/**
  *  \file u/t_game_map_shipinfo.cpp
  *  \brief Test for game::map::ShipInfo
  */

#include "game/map/shipinfo.hpp"

#include "t_game_map.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/configuration.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/sys/log.hpp"

using game::map::Point;
using game::map::Ship;

namespace {
    void addShipTrack(Ship& ship, int turnNr, Point pos)
    {
        game::parser::MessageInformation mi(game::parser::MessageInformation::Ship, ship.getId(), turnNr);
        mi.addValue(game::parser::mi_X, pos.getX());
        mi.addValue(game::parser::mi_Y, pos.getY());
        mi.addValue(game::parser::mi_Mass, 100);
        ship.addMessageInformation(mi, game::PlayerSet_t());
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

