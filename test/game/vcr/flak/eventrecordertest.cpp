/**
  *  \file test/game/vcr/flak/eventrecordertest.cpp
  *  \brief Test for game::vcr::flak::EventRecorder
  */

#include "game/vcr/flak/eventrecorder.hpp"

#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    using afl::string::Format;
    using game::vcr::flak::Position;

    String_t toString(const Position& pos)
    {
        return Format("%d,%d,%d", pos.x, pos.y, pos.z);
    }


    class Tester : public game::vcr::flak::Visualizer, public afl::test::CallReceiver {
     public:
        Tester(afl::test::Assert a)
            : Visualizer(), CallReceiver(a)
            { }
        virtual void updateTime(int32_t time)
            { checkCall(Format("updateTime(%d)", time)); }
        virtual void fireBeamFighterFighter(Object_t from, Object_t to, bool hits)
            { checkCall(Format("fireBeamFighterFighter(%d,%d,%d)", from, to, int(hits))); }
        virtual void fireBeamFighterShip(Object_t from, Ship_t to, bool hits)
            { checkCall(Format("fireBeamFighterShip(%d,%d,%d)", from, to, int(hits))); }
        virtual void fireBeamShipFighter(Ship_t from, int beamNr, Object_t to, bool hits)
            { checkCall(Format("fireBeamShipFighter(%d,%d,%d,%d)", from, beamNr, to, int(hits))); }
        virtual void fireBeamShipShip(Ship_t from, int beamNr, Ship_t to, bool hits)
            { checkCall(Format("fireBeamShipShip(%d,%d,%d,%d)", from, beamNr, to, int(hits))); }
        virtual void createFighter(Object_t id, const Position& pos, int player, Ship_t enemy)
            { checkCall(Format("createFighter(%d,%s,%d,%d)", id, toString(pos), player, enemy)); }
        virtual void killFighter(Object_t id)
            { checkCall(Format("killFighter(%d)", id)); }
        virtual void landFighter(Object_t id)
            { checkCall(Format("landFighter(%d)", id)); }
        virtual void moveFighter(Object_t id, const Position& pos, Ship_t to)
            { checkCall(Format("moveFighter(%d,%s,%d)", id, toString(pos), to)); }
        virtual void createFleet(Fleet_t fleetNr, int32_t x, int32_t y, int player, Ship_t firstShip, size_t numShips)
            { checkCall(Format("createFleet(%d,%d,%d,%d,%d,%d)") << fleetNr << x << y << player << firstShip << numShips); }
        virtual void setEnemy(Fleet_t fleetNr, Ship_t enemy)
            { checkCall(Format("setEnemy(%d,%d)", fleetNr, enemy)); }
        virtual void killFleet(Fleet_t fleetNr)
            { checkCall(Format("killFleet(%d)", fleetNr)); }
        virtual void moveFleet(Fleet_t fleetNr, int32_t x, int32_t y)
            { checkCall(Format("moveFleet(%d,%d,%d)", fleetNr, x, y)); }
        virtual void createShip(Ship_t shipNr, const Position& pos, const ShipInfo& info)
            { checkCall(Format("createShip(%d,%s,%d,%d)", shipNr, toString(pos), info.player, int(info.isPlanet))); }
        virtual void killShip(Ship_t shipNr)
            { checkCall(Format("killShip(%d)", shipNr)); }
        virtual void moveShip(Ship_t shipNr, const Position& pos)
            { checkCall(Format("moveShip(%d,%s)", shipNr, toString(pos))); }
        virtual void createTorpedo(Object_t id, const Position& pos, int player, Ship_t enemy)
            { checkCall(Format("createTorpedo(%d,%s,%d,%d)", id, toString(pos), player, enemy)); }
        virtual void hitTorpedo(Object_t id, Ship_t shipNr)
            { checkCall(Format("hitTorpedo(%d,%d)", id, shipNr)); }
        virtual void missTorpedo(Object_t id)
            { checkCall(Format("missTorpedo(%d)", id)); }
        virtual void moveTorpedo(Object_t id, const Position& pos)
            { checkCall(Format("moveTorpedo(%d,%s)", id, toString(pos))); }
    };
}

/** Test parameter passing for all methods. */
AFL_TEST("game.vcr.flak.EventRecorder:basics", a)
{
    game::vcr::flak::EventRecorder testee;
    Tester t(a);

    testee.updateTime(7788);
    t.expectCall("updateTime(7788)");

    testee.fireBeamFighterFighter(10, 99, true);
    t.expectCall("fireBeamFighterFighter(10,99,1)");

    testee.fireBeamFighterShip(20,30,false);
    t.expectCall("fireBeamFighterShip(20,30,0)");

    testee.fireBeamShipFighter(11, 2, 44, true);
    t.expectCall("fireBeamShipFighter(11,2,44,1)");

    testee.fireBeamShipShip(12, 1, 9, false);
    t.expectCall("fireBeamShipShip(12,1,9,0)");

    testee.createFighter(72, Position(1000,2000,3000), 4, 10);
    t.expectCall("createFighter(72,1000,2000,3000,4,10)");

    testee.killFighter(74);
    t.expectCall("killFighter(74)");

    testee.landFighter(75);
    t.expectCall("landFighter(75)");

    testee.moveFighter(72, Position(1100,2300,3400), 15);
    t.expectCall("moveFighter(72,1100,2300,3400,15)");

    testee.createFleet(70, 4000, 5000, 3, 5, 2);
    t.expectCall("createFleet(70,4000,5000,3,5,2)");

    testee.setEnemy(70, 100);
    t.expectCall("setEnemy(70,100)");

    testee.killFleet(42);
    t.expectCall("killFleet(42)");

    testee.moveFleet(45, 40000, -30000);
    t.expectCall("moveFleet(45,40000,-30000)");

    game::vcr::flak::Visualizer::ShipInfo info;
    info.player = 10;
    info.isPlanet = true;
    testee.createShip(50, Position(-50000,40000,200), info);
    t.expectCall("createShip(50,-50000,40000,200,10,1)");

    testee.killShip(50);
    t.expectCall("killShip(50)");

    testee.moveShip(51, Position(55,44,33));
    t.expectCall("moveShip(51,55,44,33)");

    testee.createTorpedo(555, Position(1000,8000,9000), 12, 70);
    t.expectCall("createTorpedo(555,1000,8000,9000,12,70)");

    testee.hitTorpedo(47, 200);
    t.expectCall("hitTorpedo(47,200)");

    testee.missTorpedo(48);
    t.expectCall("missTorpedo(48)");

    testee.moveTorpedo(49, Position(400,500,300));
    t.expectCall("moveTorpedo(49,400,500,300)");

    testee.replay(t);
    t.checkFinish();
}

/** Test swap() and related. */
AFL_TEST("game.vcr.flak.EventRecorder:swap", a)
{
    // Create a Recorder. Must be empty on start.
    game::vcr::flak::EventRecorder ra;
    a.checkEqual("01. size", ra.size(), 0U);

    // Add one call
    ra.killShip(1);
    a.checkGreaterThan("11. size", ra.size(), 0U);

    // Replay
    {
        Tester t("first replay");
        t.expectCall("killShip(1)");
        AFL_CHECK_SUCCEEDS(a("21. replay"), ra.replay(t));
        t.checkFinish();
    }

    // Swap
    util::StringInstructionList sil;
    ra.swapContent(sil);
    a.checkEqual("31. size", ra.size(), 0U);

    // Replay now produces nothing
    {
        Tester t("second replay");
        AFL_CHECK_SUCCEEDS(a("41. replay"), ra.replay(t));
        t.checkFinish();
    }

    // Replay using another recorder
    game::vcr::flak::EventRecorder rb;
    rb.swapContent(sil);
    {
        Tester t("third replay");
        t.expectCall("killShip(1)");
        AFL_CHECK_SUCCEEDS(a("51. replay"), rb.replay(t));
        t.checkFinish();
    }
}
