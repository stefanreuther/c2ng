/**
  *  \file u/t_game_map_shiputils.cpp
  *  \brief Test for game::map::ShipUtils
  */

#include "game/map/shiputils.hpp"

#include "t_game_map.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/map/shipstorage.hpp"

using game::config::HostConfiguration;
using game::map::Configuration;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;
using game::map::Universe;

namespace {
    Ship& addPlayedShip(Universe& univ, game::Id_t id, Point pos, String_t fc)
    {
        Ship& sh = *univ.ships().create(id);
        game::map::ShipData sd;
        sd.x = pos.getX();
        sd.y = pos.getY();
        sd.friendlyCode = fc;
        sd.owner = 3;
        sh.addCurrentShipData(sd, game::PlayerSet_t(3));
        sh.internalCheck();
        sh.combinedCheck1(univ, game::PlayerSet_t(3), 10);
        sh.setPlayability(game::map::Object::Playable);
        return sh;
    }
}

/** Test getShipMission(), various scenarios. */
void
TestGameMapShipUtils::testGetShipMission()
{
    HostConfiguration config;
    game::spec::MissionList missions;
    missions.addMission(game::spec::Mission(5, ",Sensor"));

    // Unknown mission
    {
        Ship sh(10);
        TS_ASSERT(getShipMission(sh, config, missions) == 0);
    }

    // Mission known but not defined
    {
        Ship sh(10);
        sh.setMission(10, 0, 0);
        TS_ASSERT(getShipMission(sh, config, missions) == 0);
    }

    // Mission known and defined, but no owner
    {
        Ship sh(10);
        sh.setMission(5, 0, 0);
        TS_ASSERT(getShipMission(sh, config, missions) == 0);
    }

    // Mission known and defined
    {
        Ship sh(10);
        sh.setMission(5, 0, 0);
        sh.setOwner(3);
        const game::spec::Mission* msn = getShipMission(sh, config, missions);
        TS_ASSERT(msn != 0);
        TS_ASSERT_EQUALS(msn->getNumber(), 5);
    }
}

/** Test getShipMissionByNumber(), various scenarios. */
void
TestGameMapShipUtils::testGetShipMissionByNumber()
{
    HostConfiguration config;
    game::spec::MissionList missions;
    missions.addMission(game::spec::Mission(5, ",Sensor"));
    missions.addMission(game::spec::Mission(9, "+4,Four"));
    missions.addMission(game::spec::Mission(9, "+5,Five"));
    config[HostConfiguration::PlayerSpecialMission].set("5,5,5,5,5,5,3,3,3");

    // Owner not known
    {
        Ship sh(10);
        TS_ASSERT(getShipMissionByNumber(5, sh, config, missions) == 0);
    }

    // Mission not defined
    {
        Ship sh(10);
        sh.setOwner(4);
        TS_ASSERT(getShipMissionByNumber(7, sh, config, missions) == 0);
    }

    // Mission known and defined
    {
        Ship sh(10);
        sh.setOwner(4);
        const game::spec::Mission* msn = getShipMissionByNumber(5, sh, config, missions);
        TS_ASSERT(msn != 0);
        TS_ASSERT_EQUALS(msn->getNumber(), 5);
    }

    // Race mapping
    {
        Ship sh(10);
        sh.setOwner(2);
        const game::spec::Mission* msn = getShipMissionByNumber(9, sh, config, missions);
        TS_ASSERT(msn != 0);
        TS_ASSERT_EQUALS(msn->getNumber(), 9);
        TS_ASSERT_EQUALS(msn->getName(), "Five");
    }
}

/** Test setInterceptWaypoint(), various scenarios. */
void
TestGameMapShipUtils::testSetInterceptWaypoint()
{
    // Standard case
    {
        Universe univ;
        Configuration mapConfig;

        Ship& a = *univ.ships().create(10);
        a.setPosition(Point(1200, 1300));
        a.setMission(8, 30, 0);

        Ship& b = *univ.ships().create(30);
        b.setPosition(Point(1300, 1320));

        setInterceptWaypoint(univ, a, mapConfig);

        TS_ASSERT_EQUALS(a.getWaypointDX().orElse(0), 100);
        TS_ASSERT_EQUALS(a.getWaypointDY().orElse(0), 20);
    }

    // Wraparound
    {
        Universe univ;
        Configuration mapConfig;
        mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

        Ship& a = *univ.ships().create(10);
        a.setPosition(Point(1100, 1200));
        a.setMission(8, 30, 0);

        Ship& b = *univ.ships().create(30);
        b.setPosition(Point(2900, 1300));

        setInterceptWaypoint(univ, a, mapConfig);

        TS_ASSERT_EQUALS(a.getWaypointDX().orElse(0), -200);
        TS_ASSERT_EQUALS(a.getWaypointDY().orElse(0), 100);
    }

    // Target position not known
    {
        Universe univ;
        Configuration mapConfig;

        Ship& a = *univ.ships().create(10);
        a.setPosition(Point(1100, 1200));
        a.setMission(8, 30, 0);

        Ship& b = *univ.ships().create(30);

        setInterceptWaypoint(univ, a, mapConfig);

        TS_ASSERT_EQUALS(a.getWaypointDX().isValid(), false);
        TS_ASSERT_EQUALS(a.getWaypointDY().isValid(), false);
    }
}

/** Test cancelAllCloneOrders(). */
void
TestGameMapShipUtils::testCancelAllCloneOrders()
{
    // Setup/environment
    const Point pos(2100, 2300);
    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    Universe univ;
    Planet& p = *univ.planets().create(100);
    p.setPosition(pos);

    Ship& a = addPlayedShip(univ, 10, pos, "cln");
    Ship& b = addPlayedShip(univ, 20, pos, "xyz");
    Ship& c = addPlayedShip(univ, 30, pos + Point(1,1), "cln");
    Ship& d = addPlayedShip(univ, 40, pos, "cln");

    // Test
    cancelAllCloneOrders(univ, p, fcl, rng);

    TS_ASSERT_DIFFERS(a.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(b.getFriendlyCode().orElse(""), "xyz");
    TS_ASSERT_EQUALS(c.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_DIFFERS(d.getFriendlyCode().orElse(""), "cln");
}

/** Test getShipHull(), various scenarios. */
void
TestGameMapShipUtils::testGetShipHull()
{
    game::spec::ShipList sl;
    sl.hulls().create(20);

    // Hull not known
    {
        Ship sh(10);
        TS_ASSERT(getShipHull(sh, sl) == 0);
    }

    // Hull known but not defined
    {
        Ship sh(10);
        sh.setHull(40);
        TS_ASSERT(getShipHull(sh, sl) == 0);
    }

    // Hull known and defined
    {
        Ship sh(10);
        sh.setHull(20);
        TS_ASSERT(getShipHull(sh, sl) != 0);
        TS_ASSERT_EQUALS(getShipHull(sh, sl)->getId(), 20);
    }
}

/** Test getShipTransferMaxCargo(). */
void
TestGameMapShipUtils::testGetShipTransferMaxCargo()
{
    game::spec::ShipList sl;
    game::spec::Hull& h = *sl.hulls().create(20);
    h.setMaxCargo(300);
    h.setMaxFuel(50);

    // Standard case
    {
        Ship sh(10);
        sh.setHull(20);
        sh.setCargo(game::Element::Neutronium, 20);
        sh.setCargo(game::Element::Tritanium,  10);
        sh.setCargo(game::Element::Duranium,    5);
        sh.setCargo(game::Element::Molybdenum,  7);
        sh.setCargo(game::Element::Colonists,  10);
        sh.setCargo(game::Element::Supplies,   15);
        sh.setCargo(game::Element::Money,     500);
        sh.setTorpedoType(3);
        sh.setAmmo(4);
        sh.setNumLaunchers(1);
        sh.setPlayability(game::map::Object::Playable);
        // -> total cargo is 10+5+7+10+15+4 = 51

        game::map::ShipStorage cc(sh, sl);

        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 50);   // same as hull
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 259);  // cargo minus everything but T
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 254);  // cargo minus everything but D
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::fromTorpedoType(3), sh, sl), 253);  // cargo minus everything but torps
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);

        // Turn on overload
        cc.setOverload(true);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 10000);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 10000);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 10000);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::fromTorpedoType(3), sh, sl), 10000);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);
    }

    // Same thing, but no weapons
    {
        Ship sh(10);
        sh.setHull(20);
        sh.setCargo(game::Element::Neutronium, 20);
        sh.setCargo(game::Element::Tritanium,  10);
        sh.setCargo(game::Element::Duranium,    5);
        sh.setCargo(game::Element::Molybdenum,  7);
        sh.setCargo(game::Element::Colonists,  10);
        sh.setCargo(game::Element::Supplies,   15);
        sh.setCargo(game::Element::Money,     500);
        sh.setAmmo(4);                                           // Value is ignored because it has no meaning
        sh.setPlayability(game::map::Object::Playable);
        // -> total cargo is 10+5+7+10+15 = 47

        game::map::ShipStorage cc(sh, sl);

        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 50);   // same as hull
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 263);  // cargo minus everything but T
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 258);  // cargo minus everything but D
    }

    // Hull not known
    {
        Ship sh(10);
        sh.setCargo(game::Element::Neutronium, 20);
        sh.setCargo(game::Element::Tritanium,  10);
        sh.setCargo(game::Element::Duranium,    5);
        sh.setCargo(game::Element::Molybdenum,  7);
        sh.setCargo(game::Element::Colonists,  10);
        sh.setCargo(game::Element::Supplies,   15);
        sh.setCargo(game::Element::Money,     500);
        sh.setPlayability(game::map::Object::Playable);

        game::map::ShipStorage cc(sh, sl);

        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 0);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), -37);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), -42);
        TS_ASSERT_EQUALS(getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);
    }
}

