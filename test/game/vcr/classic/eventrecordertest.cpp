/**
  *  \file test/game/vcr/classic/eventrecordertest.cpp
  *  \brief Test for game::vcr::classic::EventRecorder
  */

#include "game/vcr/classic/eventrecorder.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    using afl::string::Format;
    using game::vcr::classic::Side;
    using game::vcr::classic::LeftSide;
    using game::vcr::classic::RightSide;

    class Tester : public game::vcr::classic::EventListener, public afl::test::CallReceiver {
     public:
        Tester(afl::test::Assert a)
            : EventListener(), CallReceiver(a)
            { }
        virtual void placeObject(Side side, const UnitInfo& info)
            { checkCall(Format("placeObject(%d,'%s')") << int(side) << info.object.getName()); }
        virtual void updateTime(game::vcr::classic::Time_t time, int32_t distance)
            { checkCall(Format("updateTime(%d,%d)") << time << distance); }
        virtual void startFighter(Side side, int track, int position, int distance, int fighterDiff)
            { checkCall(Format("startFighter(%d,%d,%d,%d,%d)") << int(side) << track << position << distance << fighterDiff); }
        virtual void landFighter(Side side, int track, int fighterDiff)
            { checkCall(Format("landFighter(%d,%d,%d)") << int(side) << track << fighterDiff); }
        virtual void killFighter(Side side, int track)
            { checkCall(Format("killFighter(%d,%d)") << int(side) << track); }
        virtual void fireBeam(Side side, int track, int /*target*/, int /*hit*/, int /*damage*/, int /*kill*/, const HitEffect& effect)
            { checkCall(Format("fireBeam(%d,%d...,%d)") << int(side) << track << effect.damageDone); }
        virtual void fireTorpedo(Side side, int hit, int launcher, int torpedoDiff, const HitEffect& effect)
            { checkCall(Format("fireTorpedo(%d,%d,%d,%d,%d)") << int(side) << hit << launcher << torpedoDiff << effect.damageDone); }
        virtual void updateBeam(Side side, int id, int value)
            { checkCall(Format("updateBeam(%d,%d,%d)") << int(side) << id << value); }
        virtual void updateLauncher(Side side, int id, int value)
            { checkCall(Format("updateLauncher(%d,%d,%d)") << int(side) << id << value); }
        virtual void moveObject(Side side, int position)
            { checkCall(Format("moveObject(%d,%d)") << int(side) << position); }
        virtual void moveFighter(Side side, int track, int position, int distance, game::vcr::classic::FighterStatus status)
            { checkCall(Format("moveFighter(%d,%d,%d,%d,%d)") << int(side) << track << position << distance << int(status)); }
        virtual void killObject(Side side)
            { checkCall(Format("killObject(%d)") << int(side)); }
        virtual void updateObject(Side side, int damage, int crew, int shield)
            { checkCall(Format("updateObject(%d,%d,%d,%d)") << int(side) << damage << crew << shield); }
        virtual void updateAmmo(Side side, int numTorpedoes, int numFighters)
            { checkCall(Format("updateAmmo(%d,%d,%d)") << int(side) << numTorpedoes << numFighters); }
        virtual void updateFighter(Side side, int track, int position, int distance, game::vcr::classic::FighterStatus status)
            { checkCall(Format("updateFighter(%d,%d,%d,%d,%d)") << int(side) << track << position << distance << int(status)); }
        virtual void setResult(game::vcr::classic::BattleResult_t result)
            { checkCall(Format("setResult(%d)") << result.toInteger()); }
        virtual void removeAnimations()
            { checkCall("removeAnimations()"); }
    };
}

/** Simple test. */
AFL_TEST("game.vcr.classic.EventRecorder:basics", a)
{
    static_assert(int(game::vcr::classic::FighterAttacks) == 1, "FighterAttacks");
    static_assert(int(game::vcr::classic::FighterReturns) == 2, "FighterReturns");
    static_assert(int(LeftSide) == 0, "LeftSide");
    static_assert(int(RightSide) == 1, "RightSide");

    // Prepare some calls
    game::vcr::classic::EventRecorder testee;
    Tester t(a);
    {
        game::vcr::classic::EventListener::UnitInfo ui;
        ui.object.setName("USS Test");
        testee.placeObject(LeftSide, ui);
        t.expectCall("placeObject(0,'USS Test')");
    }

    testee.updateTime(99, 40000);
    t.expectCall("updateTime(99,40000)");

    testee.startFighter(RightSide, 17, 99, 42, -2);
    t.expectCall("startFighter(1,17,99,42,-2)");

    testee.landFighter(LeftSide, 12, 1);
    t.expectCall("landFighter(0,12,1)");

    testee.killFighter(RightSide, 9);
    t.expectCall("killFighter(1,9)");

    {
        game::vcr::classic::EventListener::HitEffect eff;
        eff.damageDone = 32;
        testee.fireBeam(LeftSide, 1, 2, 3, 4, 5, eff);
        t.expectCall("fireBeam(0,1...,32)");
    }

    {
        game::vcr::classic::EventListener::HitEffect eff;
        eff.damageDone = 92;
        testee.fireTorpedo(RightSide, 3, 4, 5, eff);
        t.expectCall("fireTorpedo(1,3,4,5,92)");
    }

    testee.updateBeam(RightSide, 9, 82);
    t.expectCall("updateBeam(1,9,82)");

    testee.updateLauncher(LeftSide, 3, 2);
    t.expectCall("updateLauncher(0,3,2)");

    testee.moveObject(RightSide, 28000);
    t.expectCall("moveObject(1,28000)");

    testee.moveFighter(LeftSide, 7, -10000, 350, game::vcr::classic::FighterAttacks);
    t.expectCall("moveFighter(0,7,-10000,350,1)");

    testee.killObject(RightSide);
    t.expectCall("killObject(1)");

    testee.updateObject(LeftSide, 75, 250, 3);
    t.expectCall("updateObject(0,75,250,3)");

    testee.updateAmmo(RightSide, 15, 9);
    t.expectCall("updateAmmo(1,15,9)");

    testee.updateFighter(RightSide, 12, 8000, 4000, game::vcr::classic::FighterReturns);
    t.expectCall("updateFighter(1,12,8000,4000,2)");

    testee.setResult(game::vcr::classic::BattleResult_t::fromInteger(12));
    t.expectCall("setResult(12)");

    testee.removeAnimations();
    t.expectCall("removeAnimations()");

    // Verify
    AFL_CHECK_SUCCEEDS(a("replay"), testee.replay(t));
    t.checkFinish();
}

/** Test swap(). */
AFL_TEST("game.vcr.classic.EventRecorder:swap", a)
{
    // Create a Recorder. Must be empty on start.
    game::vcr::classic::EventRecorder ra;
    a.checkEqual("01. size", ra.size(), 0U);

    // Add one call
    ra.killObject(RightSide);
    a.checkGreaterThan("11. size", ra.size(), 0U);

    // Replay
    {
        Tester t(a("first replay"));
        t.expectCall("killObject(1)");
        AFL_CHECK_SUCCEEDS(a("21. replay"), ra.replay(t));
        t.checkFinish();
    }

    // Swap
    util::StringInstructionList sil;
    ra.swapContent(sil);
    a.checkEqual("31. size", ra.size(), 0U);

    // Replay now produces nothing
    {
        Tester t(a("second replay"));
        AFL_CHECK_SUCCEEDS(a("41. replay"), ra.replay(t));
        t.checkFinish();
    }

    // Replay using another recorder
    game::vcr::classic::EventRecorder rb;
    rb.swapContent(sil);
    {
        Tester t(a("third replay"));
        t.expectCall("killObject(1)");
        AFL_CHECK_SUCCEEDS(a("51. replay"), rb.replay(t));
        t.checkFinish();
    }
}
