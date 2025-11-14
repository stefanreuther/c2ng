/**
  *  \file test/game/map/shiputilstest.cpp
  *  \brief Test for game::map::ShipUtils
  */

#include "game/map/shiputils.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipstorage.hpp"
#include "game/map/universe.hpp"

using afl::base::Ref;
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
        sh.internalCheck(game::PlayerSet_t(3), 10);
        sh.setPlayability(game::map::Object::Playable);
        return sh;
    }
}

/*
 *  getShipMission
 */

namespace {
    void prepareOneMission(game::spec::MissionList& missions)
    {
        missions.addMission(game::spec::Mission(5, ",Sensor"));
    }
}

/** Test getShipMission(), various scenarios. */

// Unknown mission
AFL_TEST("game.map.ShipUtils:getShipMission:unknown", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();

    Ship sh(10);
    a.checkNull("", getShipMission(sh, *config, *missions));
}

// Mission known but not defined
AFL_TEST("game.map.ShipUtils:getShipMission:undefined", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();

    Ship sh(10);
    sh.setMission(10, 0, 0);
    a.checkNull("", getShipMission(sh, *config, *missions));
}

// Mission known and defined, but no owner
AFL_TEST("game.map.ShipUtils:getShipMission:no-owner", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareOneMission(*missions);

    Ship sh(10);
    sh.setMission(5, 0, 0);
    a.checkNull("", getShipMission(sh, *config, *missions));
}

// Mission known and defined
AFL_TEST("game.map.ShipUtils:getShipMission:normal", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareOneMission(*missions);
    Ship sh(10);
    sh.setMission(5, 0, 0);
    sh.setOwner(3);

    const game::spec::Mission* msn = getShipMission(sh, *config, *missions);
    a.checkNonNull("getShipMission", msn);
    a.checkEqual("getNumber", msn->getNumber(), 5);
}

/*
 *  getShipMissionByNumber(), various scenarios.
 */

namespace {
    void prepareThreeMissions(HostConfiguration& config, game::spec::MissionList& missions)
    {
        missions.addMission(game::spec::Mission(5, ",Sensor"));
        missions.addMission(game::spec::Mission(9, "+4,Four"));
        missions.addMission(game::spec::Mission(9, "+5,Five"));
        config[HostConfiguration::PlayerSpecialMission].set("5,5,5,5,5,5,3,3,3");
    }
}

// Owner not known
AFL_TEST("game.map.ShipUtils:getShipMissionByNumber:unknown-owner", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareThreeMissions(*config, *missions);

    Ship sh(10);
    a.checkNull("", getShipMissionByNumber(5, sh, *config, *missions));
}

// Mission not defined
AFL_TEST("game.map.ShipUtils:getShipMissionByNumber:undefined", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareThreeMissions(*config, *missions);

    Ship sh(10);
    sh.setOwner(4);
    a.checkNull("", getShipMissionByNumber(7, sh, *config, *missions));
}

// Mission known and defined
AFL_TEST("game.map.ShipUtils:getShipMissionByNumber:normal", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareThreeMissions(*config, *missions);

    Ship sh(10);
    sh.setOwner(4);
    const game::spec::Mission* msn = getShipMissionByNumber(5, sh, *config, *missions);
    a.checkNonNull("getShipMissionByNumber", msn);
    a.checkEqual("getNumber", msn->getNumber(), 5);
}

// Race mapping
AFL_TEST("game.map.ShipUtils:getShipMissionByNumber:race-mapping", a)
{
    Ref<HostConfiguration> config = HostConfiguration::create();
    Ref<game::spec::MissionList> missions = game::spec::MissionList::create();
    prepareThreeMissions(*config, *missions);

    Ship sh(10);
    sh.setOwner(2);
    const game::spec::Mission* msn = getShipMissionByNumber(9, sh, *config, *missions);
    a.checkNonNull("getShipMissionByNumber", msn);
    a.checkEqual("getNumber", msn->getNumber(), 9);
    a.checkEqual("getName", msn->getName(), "Five");
}

/*
 *  setInterceptWaypoint(), various scenarios.
 */

// Standard case
AFL_TEST("game.map.ShipUtils:setInterceptWaypoint:normal", a)
{
    Universe univ;
    Configuration mapConfig;

    Ship& sa = *univ.ships().create(10);
    sa.setPosition(Point(1200, 1300));
    sa.setMission(8, 30, 0);

    Ship& sb = *univ.ships().create(30);
    sb.setPosition(Point(1300, 1320));

    setInterceptWaypoint(univ, sa, mapConfig);

    a.checkEqual("getWaypointDX", sa.getWaypointDX().orElse(0), 100);
    a.checkEqual("getWaypointDY", sa.getWaypointDY().orElse(0), 20);
}

// Wraparound
AFL_TEST("game.map.ShipUtils:setInterceptWaypoint:wrap", a)
{
    Universe univ;
    Configuration mapConfig;
    mapConfig.setConfiguration(Configuration::Wrapped, Point(2000, 2000), Point(2000, 2000));

    Ship& sa = *univ.ships().create(10);
    sa.setPosition(Point(1100, 1200));
    sa.setMission(8, 30, 0);

    Ship& sb = *univ.ships().create(30);
    sb.setPosition(Point(2900, 1300));

    setInterceptWaypoint(univ, sa, mapConfig);

    a.checkEqual("getWaypointDX", sa.getWaypointDX().orElse(0), -200);
    a.checkEqual("getWaypointDY", sa.getWaypointDY().orElse(0), 100);
}

// Target position not known
AFL_TEST("game.map.ShipUtils:setInterceptWaypoint:unknown-target", a)
{
    Universe univ;
    Configuration mapConfig;

    Ship& sa = *univ.ships().create(10);
    sa.setPosition(Point(1100, 1200));
    sa.setMission(8, 30, 0);

    /*Ship& sb =*/ univ.ships().create(30);

    setInterceptWaypoint(univ, sa, mapConfig);

    a.checkEqual("getWaypointDX", sa.getWaypointDX().isValid(), false);
    a.checkEqual("getWaypointDY", sa.getWaypointDY().isValid(), false);
}


/** Test cancelAllCloneOrders(). */
AFL_TEST("game.map.ShipUtils:cancelAllCloneOrders", a)
{
    // Setup/environment
    const Point pos(2100, 2300);
    game::spec::FriendlyCodeList fcl;
    util::RandomNumberGenerator rng(0);

    Universe univ;
    Planet& p = *univ.planets().create(100);
    p.setPosition(pos);

    Ship& sa = addPlayedShip(univ, 10, pos, "cln");
    Ship& sb = addPlayedShip(univ, 20, pos, "xyz");
    Ship& sc = addPlayedShip(univ, 30, pos + Point(1,1), "cln");
    Ship& sd = addPlayedShip(univ, 40, pos, "cln");

    // Test
    cancelAllCloneOrders(univ, p, fcl, rng);

    a.checkDifferent("ship 10", sa.getFriendlyCode().orElse(""), "cln");
    a.checkEqual    ("ship 20", sb.getFriendlyCode().orElse(""), "xyz");
    a.checkEqual    ("ship 30", sc.getFriendlyCode().orElse(""), "cln");
    a.checkDifferent("ship 40", sd.getFriendlyCode().orElse(""), "cln");
}

/*
 *  getShipHull(), various scenarios
 */

// Hull not known
AFL_TEST("game.map.ShipUtils:getShipHull:unknown", a)
{
    game::spec::ShipList sl;
    sl.hulls().create(20);
    Ship sh(10);
    a.checkNull("", getShipHull(sh, sl));
}

// Hull known but not defined
AFL_TEST("game.map.ShipUtils:getShipHull:undefined", a)
{
    game::spec::ShipList sl;
    sl.hulls().create(20);
    Ship sh(10);
    sh.setHull(40);
    a.checkNull("", getShipHull(sh, sl));
}

// Hull known and defined
AFL_TEST("game.map.ShipUtils:getShipHull:normal", a)
{
    game::spec::ShipList sl;
    sl.hulls().create(20);
    Ship sh(10);
    sh.setHull(20);
    a.checkNonNull("getShipHull", getShipHull(sh, sl));
    a.checkEqual("getId", getShipHull(sh, sl)->getId(), 20);
}

/*
 *  getShipTransferMaxCargo()
 */

namespace {
    void prepareHull(game::spec::ShipList& sl)
    {
        game::spec::Hull& h = *sl.hulls().create(20);
        h.setMaxCargo(300);
        h.setMaxFuel(50);
    }
}

// Standard case
AFL_TEST("game.map.ShipUtils:getShipTransferMaxCargo:normal", a)
{
    game::spec::ShipList sl;
    prepareHull(sl);
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

    a.checkEqual("01. Neutronium", getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 50);   // same as hull
    a.checkEqual("02. Tritanium",  getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 259);  // cargo minus everything but T
    a.checkEqual("03. Duranium",   getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 254);  // cargo minus everything but D
    a.checkEqual("04. Torpedoes",  getShipTransferMaxCargo(cc, game::Element::fromTorpedoType(3), sh, sl), 253);  // cargo minus everything but torps
    a.checkEqual("05. Money",      getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);

    // Turn on overload
    cc.setOverload(true);
    a.checkEqual("11. Neutronium", getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 10000);
    a.checkEqual("12. Tritanium",  getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 10000);
    a.checkEqual("13. Duranium",   getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 10000);
    a.checkEqual("14. Torpedoes",  getShipTransferMaxCargo(cc, game::Element::fromTorpedoType(3), sh, sl), 10000);
    a.checkEqual("15. Money",      getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);
}

// Same thing, but no weapons
AFL_TEST("game.map.ShipUtils:getShipTransferMaxCargo:freighter", a)
{
    game::spec::ShipList sl;
    prepareHull(sl);
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

    a.checkEqual("01. Neutronium", getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 50);   // same as hull
    a.checkEqual("02. Tritanium",  getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), 263);  // cargo minus everything but T
    a.checkEqual("03. Duranium",   getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), 258);  // cargo minus everything but D
}

// Hull not known
AFL_TEST("game.map.ShipUtils:getShipTransferMaxCargo:unknown", a)
{
    game::spec::ShipList sl;
    prepareHull(sl);
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

    a.checkEqual("01. Neutronium", getShipTransferMaxCargo(cc, game::Element::Neutronium, sh, sl), 0);
    a.checkEqual("02. Tritanium",  getShipTransferMaxCargo(cc, game::Element::Tritanium,  sh, sl), -37);
    a.checkEqual("03. Duranium",   getShipTransferMaxCargo(cc, game::Element::Duranium,   sh, sl), -42);
    a.checkEqual("04. Money",      getShipTransferMaxCargo(cc, game::Element::Money,      sh, sl), 10000);
}
