/**
  *  \file u/t_game_vcr_classic_mirroringeventlistener.cpp
  *  \brief Test for game::vcr::classic::MirroringEventListener
  */

#include "game/vcr/classic/mirroringeventlistener.hpp"

#include "t_game_vcr_classic.hpp"
#include "afl/test/callreceiver.hpp"
#include "game/vcr/classic/eventlistener.hpp"
#include "afl/base/staticassert.hpp"
#include "game/vcr/classic/algorithm.hpp"

namespace gvc = game::vcr::classic;

namespace {
    using afl::string::Format;
    using gvc::Side;
    using gvc::LeftSide;
    using gvc::RightSide;

    class Tester : public gvc::EventListener, public afl::test::CallReceiver {
     public:
        Tester()
            : EventListener(), CallReceiver("testIt")
            { }
        virtual void placeObject(Side side, const UnitInfo& info)
            { checkCall(Format("placeObject(%d,'%s')") << int(side) << info.object.getName()); }
        virtual void updateTime(gvc::Time_t time, int32_t distance)
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
        virtual void moveFighter(Side side, int track, int position, int distance, gvc::FighterStatus status)
            { checkCall(Format("moveFighter(%d,%d,%d,%d,%d)") << int(side) << track << position << distance << int(status)); }
        virtual void killObject(Side side)
            { checkCall(Format("killObject(%d)") << int(side)); }
        virtual void updateObject(Side side, int damage, int crew, int shield)
            { checkCall(Format("updateObject(%d,%d,%d,%d)") << int(side) << damage << crew << shield); }
        virtual void updateAmmo(Side side, int numTorpedoes, int numFighters)
            { checkCall(Format("updateAmmo(%d,%d,%d)") << int(side) << numTorpedoes << numFighters); }
        virtual void updateFighter(Side side, int track, int position, int distance, gvc::FighterStatus status)
            { checkCall(Format("updateFighter(%d,%d,%d,%d,%d)") << int(side) << track << position << distance << int(status)); }
        virtual void setResult(gvc::BattleResult_t result)
            { checkCall(Format("setResult(%d)") << result.toInteger()); }
    };
}

/** Basic functionality test. */
void
TestGameVcrClassicMirroringEventListener::testIt()
{
    static_assert(int(gvc::Algorithm::MAX_COORDINATE) == 640, "MAX_COORDINATE");

    Tester t;
    gvc::MirroringEventListener testee(t);

    // Prepare some calls
    {
        gvc::EventListener::UnitInfo ui;
        ui.object.setName("USS Test");
        t.expectCall("placeObject(1,'USS Test')");
        testee.placeObject(LeftSide, ui);
    }

    t.expectCall("updateTime(99,40000)");
    testee.updateTime(99, 40000);

    t.expectCall("startFighter(0,17,541,42,-2)");
    testee.startFighter(RightSide, 17, 99, 42, -2);

    t.expectCall("landFighter(1,12,1)");
    testee.landFighter(LeftSide, 12, 1);

    t.expectCall("killFighter(0,9)");
    testee.killFighter(RightSide, 9);

    {
        gvc::EventListener::HitEffect eff;
        eff.damageDone = 32;
        t.expectCall("fireBeam(1,1...,32)");
        testee.fireBeam(LeftSide, 1, 2, 3, 4, 5, eff);
    }

    {
        gvc::EventListener::HitEffect eff;
        eff.damageDone = 92;
        t.expectCall("fireTorpedo(0,3,4,5,92)");
        testee.fireTorpedo(RightSide, 3, 4, 5, eff);
    }

    t.expectCall("updateBeam(0,9,82)");
    testee.updateBeam(RightSide, 9, 82);

    t.expectCall("updateLauncher(1,3,2)");
    testee.updateLauncher(LeftSide, 3, 2);

    t.expectCall("moveObject(0,440)");
    testee.moveObject(RightSide, 200);

    t.expectCall("moveFighter(1,7,540,350,1)");
    testee.moveFighter(LeftSide, 7, 100, 350, gvc::FighterAttacks);

    t.expectCall("killObject(0)");
    testee.killObject(RightSide);

    t.expectCall("updateObject(1,75,250,3)");
    testee.updateObject(LeftSide, 75, 250, 3);

    t.expectCall("updateAmmo(0,15,9)");
    testee.updateAmmo(RightSide, 15, 9);

    t.expectCall("updateFighter(0,12,240,100,2)");
    testee.updateFighter(RightSide, 12, 400, 100, gvc::FighterReturns);

    static_assert(gvc::LeftDestroyed  == 0, "LeftDestroyed");
    static_assert(gvc::RightDestroyed == 1, "RightDestroyed");
    static_assert(gvc::LeftCaptured   == 2, "LeftCaptured");
    static_assert(gvc::RightCaptured  == 3, "RightCaptured");
    t.expectCall("setResult(9)");
    testee.setResult(gvc::BattleResult_t::fromInteger(6));

    static_assert(gvc::Timeout == 4, "Timeout");
    t.expectCall("setResult(16)");
    testee.setResult(gvc::BattleResult_t::fromInteger(16));

    TS_ASSERT_THROWS_NOTHING(t.checkFinish());
}

