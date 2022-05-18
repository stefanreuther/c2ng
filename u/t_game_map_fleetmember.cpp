/**
  *  \file u/t_game_map_fleetmember.cpp
  *  \brief Test for game::map::FleetMember
  */

#include "game/map/fleetmember.hpp"

#include "t_game_map.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/mutexlist.hpp"

using game::map::FleetMember;

namespace {
    /* Id to use as Intercept mission. Used to catch if anyone hardcodes the Id. */
    const int MY_INTERCEPT_MISSION = 12;

    class TestHarness {
     public:
        TestHarness()
            : univ(),
              mapConfig(),
              config(),
              shipList(),
              mutexList()
            {
                config.setDefaultValues();
                shipList.missions().addMission(game::spec::Mission(MY_INTERCEPT_MISSION, "!is*,Intercept"));
            }

        game::map::Universe univ;
        game::map::Configuration mapConfig;
        game::config::HostConfiguration config;
        game::spec::ShipList shipList;
        interpreter::MutexList mutexList;

        game::map::Ship& ship(int n)
            { return *univ.ships().get(n); }
    };

    void createShip(TestHarness& h, int id, int owner, int x, int y)
    {
        game::map::ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.owner = owner;
        sd.waypointDX = 0;
        sd.waypointDY = 0;
        sd.mission = 1;
        sd.warpFactor = 2;

        game::map::Ship& sh = *h.univ.ships().create(id);
        sh.addCurrentShipData(sd, game::PlayerSet_t(owner));

        // Make visible
        sh.internalCheck();
        sh.combinedCheck1(h.univ, game::PlayerSet_t(owner), 82);
        sh.setPlayability(game::map::Object::Playable);
    }
}


/** Test setFleetName().
    The call must be accepted only for fleet leaders, and properly be executed. */
void
TestGameMapFleetMember::testSetFleetName()
{
    // Create environment:
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    // - one fleet with one ship
    h.ship(3).setFleetNumber(3);

    // - one fleet with two ships
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test:
    // - not permitted on single ship
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetName("one"), false);

    // - permitted on single-ship fleet
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetName("three"), true);

    // - not permitted on fleet member
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setFleetName("seven"), false);

    // - permitted on leader
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setFleetName("nine"), true);

    // Verify results
    TS_ASSERT_EQUALS(h.ship(1).getFleetName(), "");
    TS_ASSERT_EQUALS(h.ship(3).getFleetName(), "three");
    TS_ASSERT_EQUALS(h.ship(7).getFleetName(), "");
    TS_ASSERT_EQUALS(h.ship(9).getFleetName(), "nine");
}

/** Test setWaypoint().
    The call must be accepted only for fleet leaders or lone ships, and properly be executed. */
void
TestGameMapFleetMember::testSetWaypoint()
{
    // Create environment:
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    // - one fleet with one ship
    h.ship(3).setFleetNumber(3);

    // - one fleet with two ships
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // - one ship is currently intercepting
    h.ship(2).setMission(MY_INTERCEPT_MISSION, 4, 0);

    // Test:
    // - permitted on single ships and one-member fleets
    game::map::Point pt(1010, 1020);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setWaypoint(pt, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(2), h.mapConfig).setWaypoint(pt, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(3), h.mapConfig).setWaypoint(pt, h.config, h.shipList), true);

    // - permitted on fleet leader but not member
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setWaypoint(pt, h.config, h.shipList), false);
    TS_ASSERT_EQUALS(*h.ship(7).getWaypoint().get(), game::map::Point(1000, 1000));

    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setWaypoint(pt, h.config, h.shipList), true);

    // Verify results
    TS_ASSERT_EQUALS(*h.ship(1).getWaypoint().get(), pt);
    TS_ASSERT_EQUALS(*h.ship(2).getWaypoint().get(), pt);
    TS_ASSERT_EQUALS(*h.ship(3).getWaypoint().get(), pt);
    TS_ASSERT_EQUALS(*h.ship(7).getWaypoint().get(), pt);
    TS_ASSERT_EQUALS(*h.ship(9).getWaypoint().get(), pt);

    TS_ASSERT_EQUALS(h.ship(2).getMission().orElse(-1), int(game::spec::Mission::msn_Explore));
}

/** Test setWarpFactor().
    The call must be accepted only for fleet leaders or lone ships, and properly be executed. */
void
TestGameMapFleetMember::testSetWarpFactor()
{
    // Create environment:
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    // - one fleet with one ship
    h.ship(3).setFleetNumber(3);

    // - one fleet with two ships
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test:
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setWarpFactor(7, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(3), h.mapConfig).setWarpFactor(7, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setWarpFactor(7, h.config, h.shipList), false);
    TS_ASSERT_EQUALS(h.ship(7).getWarpFactor().orElse(-1), 2);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setWarpFactor(7, h.config, h.shipList), true);

    // Verify results
    TS_ASSERT_EQUALS(h.ship(1).getWarpFactor().orElse(-1), 7);
    TS_ASSERT_EQUALS(h.ship(3).getWarpFactor().orElse(-1), 7);
    TS_ASSERT_EQUALS(h.ship(7).getWarpFactor().orElse(-1), 7);
    TS_ASSERT_EQUALS(h.ship(9).getWarpFactor().orElse(-1), 7);
}

/** Test setMission(), simple cases.
    The call must be accepted for standard missions on any ship. */
void
TestGameMapFleetMember::testSetMission()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(2, 44, 55, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(3, 44, 55, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(4, 44, 55, h.config, h.shipList), true);

    // Verify results
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), 2);
    TS_ASSERT_EQUALS(h.ship(7).getMission().orElse(-1), 3);
    TS_ASSERT_EQUALS(h.ship(9).getMission().orElse(-1), 4);
}

/** Test setMission(), to Intercept.
    The call must be accepted for lone ships and leaders. */
void
TestGameMapFleetMember::testSetMissionToIntercept()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 2, 0, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 3, 0, h.config, h.shipList), false);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 4, 0, h.config, h.shipList), true);

    // Verify results
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    TS_ASSERT_EQUALS(h.ship(7).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    TS_ASSERT_EQUALS(h.ship(9).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    TS_ASSERT_EQUALS(h.ship(1).getMissionParameter(game::InterceptParameter).orElse(-1), 2);
    TS_ASSERT_EQUALS(h.ship(7).getMissionParameter(game::InterceptParameter).orElse(-1), 4);   // note 4, not 3, from fleet leader!
    TS_ASSERT_EQUALS(h.ship(9).getMissionParameter(game::InterceptParameter).orElse(-1), 4);
}

/** Test setMission(), from Intercept.
    The call must be accepted for lone ships and leaders. */
void
TestGameMapFleetMember::testSetMissionFromIntercept()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);
    h.ship(1).setMission(MY_INTERCEPT_MISSION, 2, 0);
    h.ship(7).setMission(MY_INTERCEPT_MISSION, 4, 0);
    h.ship(9).setMission(MY_INTERCEPT_MISSION, 4, 0);

    // Test
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(99, 2, 0, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(99, 3, 0, h.config, h.shipList), false);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(99, 4, 0, h.config, h.shipList), true);

    // Verify results
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), 99);
    TS_ASSERT_EQUALS(h.ship(7).getMission().orElse(-1), int(game::spec::Mission::msn_Explore));
    TS_ASSERT_EQUALS(h.ship(9).getMission().orElse(-1), 99);
    TS_ASSERT_EQUALS(h.ship(1).getMissionParameter(game::InterceptParameter).orElse(-1), 2);
    TS_ASSERT_EQUALS(h.ship(7).getMissionParameter(game::InterceptParameter).orElse(-1), 0);   // note 0, not 3, from default setting!
    TS_ASSERT_EQUALS(h.ship(9).getMissionParameter(game::InterceptParameter).orElse(-1), 4);
}

/** Test setFleetNumber(), failure case. */
void
TestGameMapFleetMember::testSetFleetNumberFail()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    // - non-existant ship
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(99, h.config, h.shipList), false);

    // - existing ship that is not in a fleet
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(2, h.config, h.shipList), false);

    // - existing ship that is not a fleet leader
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(7, h.config, h.shipList), false);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 0);
}

/** Test setFleetNumber(), success case, join fleet.
    The command must be accepted and executed correctly. */
void
TestGameMapFleetMember::testSetFleetNumberSuccess()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(3).setWaypoint(game::map::Point(1111, 1222));

    // Create a new fleet
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetNumber(3, h.config, h.shipList), true);

    // Add members
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(3, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(9), h.mapConfig).setFleetNumber(3, h.config, h.shipList), true);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 3);
    TS_ASSERT_EQUALS(h.ship(3).getFleetNumber(), 3);
    TS_ASSERT_EQUALS(h.ship(9).getFleetNumber(), 3);
    TS_ASSERT_EQUALS(*h.ship(1).getWaypoint().get(), game::map::Point(1111, 1222));
    TS_ASSERT_EQUALS(*h.ship(3).getWaypoint().get(), game::map::Point(1111, 1222));
    TS_ASSERT_EQUALS(*h.ship(9).getWaypoint().get(), game::map::Point(1111, 1222));
}

/** Test setFleetNumber(), success case, dropping the leader.
    The fleet must be renamed. */
void
TestGameMapFleetMember::testSetFleetNumberDropLeader()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(4);
        h.ship(i).setWaypoint(game::map::Point(1111, 1333));
    }
    h.ship(4).setFleetName("n");

    // Remove the leader
    TS_ASSERT_EQUALS(h.ship(4).isFleetLeader(), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(4), h.mapConfig).setFleetNumber(0, h.config, h.shipList), true);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 1);      // renamed fleet Id
    TS_ASSERT_EQUALS(h.ship(3).getFleetNumber(), 1);
    TS_ASSERT_EQUALS(h.ship(4).getFleetNumber(), 0);
    TS_ASSERT_EQUALS(*h.ship(1).getWaypoint().get(), game::map::Point(1111, 1333));
    TS_ASSERT_EQUALS(*h.ship(3).getWaypoint().get(), game::map::Point(1111, 1333));
    TS_ASSERT_EQUALS(*h.ship(4).getWaypoint().get(), game::map::Point(1111, 1333));
    TS_ASSERT_EQUALS(h.ship(1).getFleetName(), "n");
    TS_ASSERT_EQUALS(h.ship(3).getFleetName(), "");
    TS_ASSERT_EQUALS(h.ship(4).getFleetName(), "");
}

/** Test setFleetNumber(), success case, dropping a member.
    The member must be removed normally, with no change to their waypoint. */
void
TestGameMapFleetMember::testSetFleetNumberDropMember()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(4);
        h.ship(i).setWaypoint(game::map::Point(1111, 1444));
    }

    // Remove a member
    TS_ASSERT_EQUALS(h.ship(7).isFleetMember(), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(7), h.mapConfig).setFleetNumber(0, h.config, h.shipList), true);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 4);
    TS_ASSERT_EQUALS(h.ship(3).getFleetNumber(), 4);
    TS_ASSERT_EQUALS(h.ship(7).getFleetNumber(), 0);
    TS_ASSERT_EQUALS(*h.ship(1).getWaypoint().get(), game::map::Point(1111, 1444));
    TS_ASSERT_EQUALS(*h.ship(3).getWaypoint().get(), game::map::Point(1111, 1444));
    TS_ASSERT_EQUALS(*h.ship(7).getWaypoint().get(), game::map::Point(1111, 1444));
}

/** Test setFleetNumber(), success case, moving a member.
    The member must be moved to the new fleet with the new waypoint. */
void
TestGameMapFleetMember::testSetFleetNumberMoveMember()
{
    TestHarness h;
    for (int i = 1; i < 3; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(2);
        h.ship(i).setWaypoint(game::map::Point(1111, 1444));
    }
    for (int i = 5; i < 9; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(5);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
    }

    // Move member
    TS_ASSERT_EQUALS(h.ship(1).isFleetMember(), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(5, h.config, h.shipList), true);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 5);
    TS_ASSERT_EQUALS(*h.ship(1).getWaypoint().get(), game::map::Point(1111, 1555));
}

/** Test setMission(), tow member case.
    Setting a tow mission to tow a fleet member must adjust the member's waypoint accordingly. */
void
TestGameMapFleetMember::testSetMissionTow()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(5);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
        h.ship(i).setWarpFactor(3);
    }

    // Set tow mission
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 3, h.config, h.shipList), true);

    // Verify: ship 3 (tow target) must have warp zero and no waypoint
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    TS_ASSERT_EQUALS(h.ship(1).getWarpFactor().orElse(-1), 3);
    TS_ASSERT_EQUALS(h.ship(3).getWaypointDX().orElse(-99), 0);
    TS_ASSERT_EQUALS(h.ship(3).getWaypointDY().orElse(-99), 0);
    TS_ASSERT_EQUALS(h.ship(3).getWarpFactor().orElse(-1), 0);

    // Clear tow mission
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(77, 0, 0, h.config, h.shipList), true);

    // Verify
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), 77);
    TS_ASSERT_EQUALS(h.ship(1).getWarpFactor().orElse(-1), 3);
    TS_ASSERT_EQUALS(*h.ship(3).getWaypoint().get(), game::map::Point(1111, 1555));
    TS_ASSERT_EQUALS(h.ship(3).getWarpFactor().orElse(-1), 3);
}

/** Test setMission(), tow other ship case.
    Setting a tow mission to tow an unrelated ship should not modify that ship. */
void
TestGameMapFleetMember::testSetMissionTowOther()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
        h.ship(i).setWarpFactor(3);
    }
    for (int i = 1; i <= 5; ++i) {
        h.ship(i).setFleetNumber(5);
    }

    // Set tow mission
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 9, h.config, h.shipList), true);

    // Verify: ship 5 (tow target) not affected
    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    TS_ASSERT_EQUALS(h.ship(1).getWarpFactor().orElse(-1), 3);
    TS_ASSERT_EQUALS(h.ship(5).getWaypointDX().orElse(-99), 111);
    TS_ASSERT_EQUALS(h.ship(5).getWaypointDY().orElse(-99), 555);
    TS_ASSERT_EQUALS(h.ship(5).getWarpFactor().orElse(-1), 3);
}

/** Test setMission(), tow invalid ship case.
    This must not crash (e.g. by accessing an invalid object). */
void
TestGameMapFleetMember::testSetMissionTowInvalid()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
        h.ship(i).setWarpFactor(3);
        h.ship(i).setFleetNumber(5);
    }

    // Set tow mission
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 777, h.config, h.shipList), true);

    TS_ASSERT_EQUALS(h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    TS_ASSERT_EQUALS(h.ship(1).getMissionParameter(game::TowParameter).orElse(-1), 777);
}

/** Test isMissionLocked().
    isMissionLocked() must behave correctly regarding fleets. */
void
TestGameMapFleetMember::testIsMissionLocked()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(2).setFleetNumber(2);
    h.ship(3).setFleetNumber(2);
    TS_ASSERT(FleetMember(h.univ, h.ship(2), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 7, 0, h.config, h.shipList));

    h.ship(5).setFleetNumber(5);
    h.ship(6).setFleetNumber(5);

    // Ship 1: non-fleet-member: not locked
    TS_ASSERT(!FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, h.config, h.shipList, h.mutexList));

    // Ship 2: fleet leader on intercept mission: not locked unless requested
    TS_ASSERT( FleetMember(h.univ, h.ship(2), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(2), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, h.config, h.shipList, h.mutexList));

    // Ship 3: fleet member on intercept mission: always locked
    TS_ASSERT( FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT( FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, h.config, h.shipList, h.mutexList));

    // Ship 5: fleet leader not on intercept mission: not locked
    TS_ASSERT(!FleetMember(h.univ, h.ship(5), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(5), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, h.config, h.shipList, h.mutexList));

    // Ship 6: fleet member not on intercept mission: not locked
    TS_ASSERT(!FleetMember(h.univ, h.ship(6), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(6), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, h.config, h.shipList, h.mutexList));
}

/** Test isMissionLocked().
    isMissionLocked() must behave correctly regarding mutexes. */
void
TestGameMapFleetMember::testIsMissionLockedMutex()
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setMission(MY_INTERCEPT_MISSION, 7, 0);
    }
    interpreter::MutexList::Mutex* mtx = h.mutexList.create("S3.WAYPOINT", "note", 0);
    TS_ASSERT(mtx != 0);

    // Ship 1: not locked
    TS_ASSERT(!FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(FleetMember::OverrideLocks, h.config, h.shipList, h.mutexList));

    // Ship 3: locked waypoint
    TS_ASSERT( FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(0,                          h.config, h.shipList, h.mutexList));
    TS_ASSERT(!FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(FleetMember::OverrideLocks, h.config, h.shipList, h.mutexList));

    mtx->removeReference();
}

/** Test setFleetNumber(), failure case, foreign ship.
    Ships of different owners cannot be put in a fleet. */
void
TestGameMapFleetMember::testSetFleetNumberForeign()
{
    TestHarness h;
    createShip(h, 1, 7, 1000, 1000);
    createShip(h, 2, 9, 1000, 1000);
    createShip(h, 3, 7, 1000, 1000);

    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(1, h.config, h.shipList), true);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(2), h.mapConfig).setFleetNumber(1, h.config, h.shipList), false);
    TS_ASSERT_EQUALS(FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetNumber(1, h.config, h.shipList), true);

    // Verify result
    TS_ASSERT_EQUALS(h.ship(1).getFleetNumber(), 1);
    TS_ASSERT_EQUALS(h.ship(2).getFleetNumber(), 0);
    TS_ASSERT_EQUALS(h.ship(3).getFleetNumber(), 1);
}

