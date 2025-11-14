/**
  *  \file test/game/map/fleetmembertest.cpp
  *  \brief Test for game::map::FleetMember
  */

#include "game/map/fleetmember.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/configuration.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/lockaccess.hpp"
#include <set>

using game::map::FleetMember;

namespace {
    /* Id to use as Intercept mission. Used to catch if anyone hardcodes the Id. */
    const int MY_INTERCEPT_MISSION = 12;

    class LockAccessMock : public interpreter::LockAccess {
     public:
        bool hasLock(const String_t& name) const
            { return m_locks.find(name) != m_locks.end(); }
        void addLock(const String_t& name)
            { m_locks.insert(name); }
     private:
        std::set<String_t> m_locks;
    };

    class TestHarness {
     public:
        TestHarness()
            : univ(),
              mapConfig(),
              config(game::config::HostConfiguration::create()),
              shipList(),
              mutexList()
            {
                config->setDefaultValues();
                shipList.missions().addMission(game::spec::Mission(MY_INTERCEPT_MISSION, "!is*,Intercept"));
            }

        game::map::Universe univ;
        game::map::Configuration mapConfig;
        afl::base::Ref<game::config::HostConfiguration> config;
        game::spec::ShipList shipList;
        LockAccessMock mutexList;

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
        sh.internalCheck(game::PlayerSet_t(owner), 82);
        sh.setPlayability(game::map::Object::Playable);
    }
}


/** Test setFleetName().
    The call must be accepted only for fleet leaders, and properly be executed. */
AFL_TEST("game.map.FleetMember:setFleetName", a)
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
    a.checkEqual("01. setFleetName", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetName("one"), false);

    // - permitted on single-ship fleet
    a.checkEqual("11. setFleetName", FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetName("three"), true);

    // - not permitted on fleet member
    a.checkEqual("21. setFleetName", FleetMember(h.univ, h.ship(7), h.mapConfig).setFleetName("seven"), false);

    // - permitted on leader
    a.checkEqual("31. setFleetName", FleetMember(h.univ, h.ship(9), h.mapConfig).setFleetName("nine"), true);

    // Verify results
    a.checkEqual("41. getFleetName", h.ship(1).getFleetName(), "");
    a.checkEqual("42. getFleetName", h.ship(3).getFleetName(), "three");
    a.checkEqual("43. getFleetName", h.ship(7).getFleetName(), "");
    a.checkEqual("44. getFleetName", h.ship(9).getFleetName(), "nine");
}

/** Test setWaypoint().
    The call must be accepted only for fleet leaders or lone ships, and properly be executed. */
AFL_TEST("game.map.FleetMember:setWaypoint", a)
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
    a.checkEqual("01. setWaypoint", FleetMember(h.univ, h.ship(1), h.mapConfig).setWaypoint(pt, *h.config, h.shipList), true);
    a.checkEqual("02. setWaypoint", FleetMember(h.univ, h.ship(2), h.mapConfig).setWaypoint(pt, *h.config, h.shipList), true);
    a.checkEqual("03. setWaypoint", FleetMember(h.univ, h.ship(3), h.mapConfig).setWaypoint(pt, *h.config, h.shipList), true);

    // - permitted on fleet leader but not member
    a.checkEqual("11. setWaypoint", FleetMember(h.univ, h.ship(7), h.mapConfig).setWaypoint(pt, *h.config, h.shipList), false);
    a.checkEqual("12. getWaypoint", *h.ship(7).getWaypoint().get(), game::map::Point(1000, 1000));

    a.checkEqual("21. setWaypoint", FleetMember(h.univ, h.ship(9), h.mapConfig).setWaypoint(pt, *h.config, h.shipList), true);

    // Verify results
    a.checkEqual("31. getWaypoint", *h.ship(1).getWaypoint().get(), pt);
    a.checkEqual("32. getWaypoint", *h.ship(2).getWaypoint().get(), pt);
    a.checkEqual("33. getWaypoint", *h.ship(3).getWaypoint().get(), pt);
    a.checkEqual("34. getWaypoint", *h.ship(7).getWaypoint().get(), pt);
    a.checkEqual("35. getWaypoint", *h.ship(9).getWaypoint().get(), pt);

    a.checkEqual("41. getMission", h.ship(2).getMission().orElse(-1), int(game::spec::Mission::msn_Explore));
}

/** Test setWarpFactor().
    The call must be accepted only for fleet leaders or lone ships, and properly be executed. */
AFL_TEST("game.map.FleetMember:setWarpFactor", a)
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
    a.checkEqual("01. setWarpFactor", FleetMember(h.univ, h.ship(1), h.mapConfig).setWarpFactor(7, *h.config, h.shipList), true);
    a.checkEqual("02. setWarpFactor", FleetMember(h.univ, h.ship(3), h.mapConfig).setWarpFactor(7, *h.config, h.shipList), true);
    a.checkEqual("03. setWarpFactor", FleetMember(h.univ, h.ship(7), h.mapConfig).setWarpFactor(7, *h.config, h.shipList), false);
    a.checkEqual("04. getWarpFactor", h.ship(7).getWarpFactor().orElse(-1), 2);
    a.checkEqual("05. setWarpFactor", FleetMember(h.univ, h.ship(9), h.mapConfig).setWarpFactor(7, *h.config, h.shipList), true);

    // Verify results
    a.checkEqual("11. getWarpFactor", h.ship(1).getWarpFactor().orElse(-1), 7);
    a.checkEqual("12. getWarpFactor", h.ship(3).getWarpFactor().orElse(-1), 7);
    a.checkEqual("13. getWarpFactor", h.ship(7).getWarpFactor().orElse(-1), 7);
    a.checkEqual("14. getWarpFactor", h.ship(9).getWarpFactor().orElse(-1), 7);
}

/** Test setMission(), simple cases.
    The call must be accepted for standard missions on any ship. */
AFL_TEST("game.map.FleetMember:setMission", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(2, 44, 55, *h.config, h.shipList), true);
    a.checkEqual("02. setMission", FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(3, 44, 55, *h.config, h.shipList), true);
    a.checkEqual("03. setMission", FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(4, 44, 55, *h.config, h.shipList), true);

    // Verify results
    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), 2);
    a.checkEqual("12. getMission", h.ship(7).getMission().orElse(-1), 3);
    a.checkEqual("13. getMission", h.ship(9).getMission().orElse(-1), 4);
}

/** Test setMission(), to Intercept.
    The call must be accepted for lone ships and leaders. */
AFL_TEST("game.map.FleetMember:setMission:to-intercept", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 2, 0, *h.config, h.shipList), true);
    a.checkEqual("02. setMission", FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 3, 0, *h.config, h.shipList), false);
    a.checkEqual("03. setMission", FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 4, 0, *h.config, h.shipList), true);

    // Verify results
    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    a.checkEqual("12. getMission", h.ship(7).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    a.checkEqual("13. getMission", h.ship(9).getMission().orElse(-1), MY_INTERCEPT_MISSION);
    a.checkEqual("14. getMissionParameter", h.ship(1).getMissionParameter(game::InterceptParameter).orElse(-1), 2);
    a.checkEqual("15. getMissionParameter", h.ship(7).getMissionParameter(game::InterceptParameter).orElse(-1), 4);   // note 4, not 3, from fleet leader!
    a.checkEqual("16. getMissionParameter", h.ship(9).getMissionParameter(game::InterceptParameter).orElse(-1), 4);
}

/** Test setMission(), from Intercept.
    The call must be accepted for lone ships and leaders. */
AFL_TEST("game.map.FleetMember:setMission:from-intercept", a)
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
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(99, 2, 0, *h.config, h.shipList), true);
    a.checkEqual("02. setMission", FleetMember(h.univ, h.ship(7), h.mapConfig).setMission(99, 3, 0, *h.config, h.shipList), false);
    a.checkEqual("03. setMission", FleetMember(h.univ, h.ship(9), h.mapConfig).setMission(99, 4, 0, *h.config, h.shipList), true);

    // Verify results
    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), 99);
    a.checkEqual("12. getMission", h.ship(7).getMission().orElse(-1), int(game::spec::Mission::msn_Explore));
    a.checkEqual("13. getMission", h.ship(9).getMission().orElse(-1), 99);
    a.checkEqual("14. getMissionParameter", h.ship(1).getMissionParameter(game::InterceptParameter).orElse(-1), 2);
    a.checkEqual("15. getMissionParameter", h.ship(7).getMissionParameter(game::InterceptParameter).orElse(-1), 0);   // note 0, not 3, from default setting!
    a.checkEqual("16. getMissionParameter", h.ship(9).getMissionParameter(game::InterceptParameter).orElse(-1), 4);
}

/** Test setFleetNumber(), failure case. */
AFL_TEST("game.map.FleetMember:setFleetNumber:fail", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(7).setFleetNumber(9);
    h.ship(9).setFleetNumber(9);

    // Test
    // - non-existant ship
    a.checkEqual("01. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(99, *h.config, h.shipList), false);

    // - existing ship that is not in a fleet
    a.checkEqual("11. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(2, *h.config, h.shipList), false);

    // - existing ship that is not a fleet leader
    a.checkEqual("21. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(7, *h.config, h.shipList), false);

    // Verify result
    a.checkEqual("31. getFleetNumber", h.ship(1).getFleetNumber(), 0);
}

/** Test setFleetNumber(), success case, join fleet.
    The command must be accepted and executed correctly. */
AFL_TEST("game.map.FleetMember:setFleetNumber:success", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(3).setWaypoint(game::map::Point(1111, 1222));

    // Create a new fleet
    a.checkEqual("01. setFleetNumber", FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetNumber(3, *h.config, h.shipList), true);

    // Add members
    a.checkEqual("11. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(3, *h.config, h.shipList), true);
    a.checkEqual("12. setFleetNumber", FleetMember(h.univ, h.ship(9), h.mapConfig).setFleetNumber(3, *h.config, h.shipList), true);

    // Verify result
    a.checkEqual("21. getFleetNumber", h.ship(1).getFleetNumber(), 3);
    a.checkEqual("22. getFleetNumber", h.ship(3).getFleetNumber(), 3);
    a.checkEqual("23. getFleetNumber", h.ship(9).getFleetNumber(), 3);
    a.checkEqual("24. getWaypoint", *h.ship(1).getWaypoint().get(), game::map::Point(1111, 1222));
    a.checkEqual("25. getWaypoint", *h.ship(3).getWaypoint().get(), game::map::Point(1111, 1222));
    a.checkEqual("26. getWaypoint", *h.ship(9).getWaypoint().get(), game::map::Point(1111, 1222));
}

/** Test setFleetNumber(), success case, dropping the leader.
    The fleet must be renamed. */
AFL_TEST("game.map.FleetMember:setFleetNumber:drop-leader", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(4);
        h.ship(i).setWaypoint(game::map::Point(1111, 1333));
    }
    h.ship(4).setFleetName("n");

    // Remove the leader
    a.checkEqual("01. isFleetLeader", h.ship(4).isFleetLeader(), true);
    a.checkEqual("02. setFleetNumber", FleetMember(h.univ, h.ship(4), h.mapConfig).setFleetNumber(0, *h.config, h.shipList), true);

    // Verify result
    a.checkEqual("11. getFleetNumber", h.ship(1).getFleetNumber(), 1);      // renamed fleet Id
    a.checkEqual("12. getFleetNumber", h.ship(3).getFleetNumber(), 1);
    a.checkEqual("13. getFleetNumber", h.ship(4).getFleetNumber(), 0);
    a.checkEqual("14. getWaypoint", *h.ship(1).getWaypoint().get(), game::map::Point(1111, 1333));
    a.checkEqual("15. getWaypoint", *h.ship(3).getWaypoint().get(), game::map::Point(1111, 1333));
    a.checkEqual("16. getWaypoint", *h.ship(4).getWaypoint().get(), game::map::Point(1111, 1333));
    a.checkEqual("17. getFleetName", h.ship(1).getFleetName(), "n");
    a.checkEqual("18. getFleetName", h.ship(3).getFleetName(), "");
    a.checkEqual("19. getFleetName", h.ship(4).getFleetName(), "");
}

/** Test setFleetNumber(), success case, dropping a member.
    The member must be removed normally, with no change to their waypoint. */
AFL_TEST("game.map.FleetMember:setFleetNumber:drop-member", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(4);
        h.ship(i).setWaypoint(game::map::Point(1111, 1444));
    }

    // Remove a member
    a.checkEqual("01. isFleetLeader", h.ship(7).isFleetMember(), true);
    a.checkEqual("02. setFleetNumber", FleetMember(h.univ, h.ship(7), h.mapConfig).setFleetNumber(0, *h.config, h.shipList), true);

    // Verify result
    a.checkEqual("11. getFleetNumber", h.ship(1).getFleetNumber(), 4);
    a.checkEqual("12. getFleetNumber", h.ship(3).getFleetNumber(), 4);
    a.checkEqual("13. getFleetNumber", h.ship(7).getFleetNumber(), 0);
    a.checkEqual("14. getWaypoint", *h.ship(1).getWaypoint().get(), game::map::Point(1111, 1444));
    a.checkEqual("15. getWaypoint", *h.ship(3).getWaypoint().get(), game::map::Point(1111, 1444));
    a.checkEqual("16. getWaypoint", *h.ship(7).getWaypoint().get(), game::map::Point(1111, 1444));
}

/** Test setFleetNumber(), success case, moving a member.
    The member must be moved to the new fleet with the new waypoint. */
AFL_TEST("game.map.FleetMember:setFleetNumber:move-member", a)
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
    a.checkEqual("01. isFleetLeader", h.ship(1).isFleetMember(), true);
    a.checkEqual("02. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(5, *h.config, h.shipList), true);

    // Verify result
    a.checkEqual("11. getFleetNumber", h.ship(1).getFleetNumber(), 5);
    a.checkEqual("12. getWaypoint", *h.ship(1).getWaypoint().get(), game::map::Point(1111, 1555));
}

/** Test setMission(), tow member case.
    Setting a tow mission to tow a fleet member must adjust the member's waypoint accordingly. */
AFL_TEST("game.map.FleetMember:setMission:tow-member", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setFleetNumber(5);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
        h.ship(i).setWarpFactor(3);
    }

    // Set tow mission
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 3, *h.config, h.shipList), true);

    // Verify: ship 3 (tow target) must have warp zero and no waypoint
    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    a.checkEqual("12. getWarpFactor", h.ship(1).getWarpFactor().orElse(-1), 3);
    a.checkEqual("13. getWaypointDX", h.ship(3).getWaypointDX().orElse(-99), 0);
    a.checkEqual("14. getWaypointDY", h.ship(3).getWaypointDY().orElse(-99), 0);
    a.checkEqual("15. getWarpFactor", h.ship(3).getWarpFactor().orElse(-1), 0);

    // Clear tow mission
    a.checkEqual("21. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(77, 0, 0, *h.config, h.shipList), true);

    // Verify
    a.checkEqual("31. getMission", h.ship(1).getMission().orElse(-1), 77);
    a.checkEqual("32. getWarpFactor", h.ship(1).getWarpFactor().orElse(-1), 3);
    a.checkEqual("33. getWaypoint", *h.ship(3).getWaypoint().get(), game::map::Point(1111, 1555));
    a.checkEqual("34. getWarpFactor", h.ship(3).getWarpFactor().orElse(-1), 3);
}

/** Test setMission(), tow other ship case.
    Setting a tow mission to tow an unrelated ship should not modify that ship. */
AFL_TEST("game.map.FleetMember:setMission:tow-other", a)
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
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 9, *h.config, h.shipList), true);

    // Verify: ship 5 (tow target) not affected
    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    a.checkEqual("12. getWarpFactor", h.ship(1).getWarpFactor().orElse(-1), 3);
    a.checkEqual("13. getWaypointDX", h.ship(5).getWaypointDX().orElse(-99), 111);
    a.checkEqual("14. getWaypointDY", h.ship(5).getWaypointDY().orElse(-99), 555);
    a.checkEqual("15. getWarpFactor", h.ship(5).getWarpFactor().orElse(-1), 3);
}

/** Test setMission(), tow invalid ship case.
    This must not crash (e.g. by accessing an invalid object). */
AFL_TEST("game.map.FleetMember:setMission:tow-invalid", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setWaypoint(game::map::Point(1111, 1555));
        h.ship(i).setWarpFactor(3);
        h.ship(i).setFleetNumber(5);
    }

    // Set tow mission
    a.checkEqual("01. setMission", FleetMember(h.univ, h.ship(1), h.mapConfig).setMission(game::spec::Mission::msn_Tow, 0, 777, *h.config, h.shipList), true);

    a.checkEqual("11. getMission", h.ship(1).getMission().orElse(-1), int(game::spec::Mission::msn_Tow));
    a.checkEqual("12. getMissionParameter", h.ship(1).getMissionParameter(game::TowParameter).orElse(-1), 777);
}

/** Test isMissionLocked().
    isMissionLocked() must behave correctly regarding fleets. */
AFL_TEST("game.map.FleetMember:isMissionLocked", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
    }
    h.ship(2).setFleetNumber(2);
    h.ship(3).setFleetNumber(2);
    a.check("01. setMission", FleetMember(h.univ, h.ship(2), h.mapConfig).setMission(MY_INTERCEPT_MISSION, 7, 0, *h.config, h.shipList));

    h.ship(5).setFleetNumber(5);
    h.ship(6).setFleetNumber(5);

    // Ship 1: non-fleet-member: not locked
    a.check("11. isMissionLocked", !FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("12. isMissionLocked", !FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, *h.config, h.shipList, h.mutexList));

    // Ship 2: fleet leader on intercept mission: not locked unless requested
    a.check("21. isMissionLocked",  FleetMember(h.univ, h.ship(2), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("22. isMissionLocked", !FleetMember(h.univ, h.ship(2), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, *h.config, h.shipList, h.mutexList));

    // Ship 3: fleet member on intercept mission: always locked
    a.check("31. isMissionLocked",  FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("32. isMissionLocked",  FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, *h.config, h.shipList, h.mutexList));

    // Ship 5: fleet leader not on intercept mission: not locked
    a.check("41. isMissionLocked", !FleetMember(h.univ, h.ship(5), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("42. isMissionLocked", !FleetMember(h.univ, h.ship(5), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, *h.config, h.shipList, h.mutexList));

    // Ship 6: fleet member not on intercept mission: not locked
    a.check("51. isMissionLocked", !FleetMember(h.univ, h.ship(6), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("52. isMissionLocked", !FleetMember(h.univ, h.ship(6), h.mapConfig).isMissionLocked(FleetMember::AcceptLeaders, *h.config, h.shipList, h.mutexList));
}

/** Test isMissionLocked().
    isMissionLocked() must behave correctly regarding mutexes. */
AFL_TEST("game.map.FleetMember:isMissionLocked:mutex", a)
{
    TestHarness h;
    for (int i = 1; i < 10; ++i) {
        createShip(h, i, 7, 1000, 1000);
        h.ship(i).setMission(MY_INTERCEPT_MISSION, 7, 0);
    }
    h.mutexList.addLock("S3.WAYPOINT");

    // Ship 1: not locked
    a.check("01. isMissionLocked", !FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("02. isMissionLocked", !FleetMember(h.univ, h.ship(1), h.mapConfig).isMissionLocked(FleetMember::OverrideLocks, *h.config, h.shipList, h.mutexList));

    // Ship 3: locked waypoint
    a.check("11. isMissionLocked",  FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(0,                          *h.config, h.shipList, h.mutexList));
    a.check("12. isMissionLocked", !FleetMember(h.univ, h.ship(3), h.mapConfig).isMissionLocked(FleetMember::OverrideLocks, *h.config, h.shipList, h.mutexList));
}

/** Test setFleetNumber(), failure case, foreign ship.
    Ships of different owners cannot be put in a fleet. */
AFL_TEST("game.map.FleetMember:setFleetNumber:foreign", a)
{
    TestHarness h;
    createShip(h, 1, 7, 1000, 1000);
    createShip(h, 2, 9, 1000, 1000);
    createShip(h, 3, 7, 1000, 1000);

    a.checkEqual("01. setFleetNumber", FleetMember(h.univ, h.ship(1), h.mapConfig).setFleetNumber(1, *h.config, h.shipList), true);
    a.checkEqual("02. setFleetNumber", FleetMember(h.univ, h.ship(2), h.mapConfig).setFleetNumber(1, *h.config, h.shipList), false);
    a.checkEqual("03. setFleetNumber", FleetMember(h.univ, h.ship(3), h.mapConfig).setFleetNumber(1, *h.config, h.shipList), true);

    // Verify result
    a.checkEqual("11. getFleetNumber", h.ship(1).getFleetNumber(), 1);
    a.checkEqual("12. getFleetNumber", h.ship(2).getFleetNumber(), 0);
    a.checkEqual("13. getFleetNumber", h.ship(3).getFleetNumber(), 1);
}
