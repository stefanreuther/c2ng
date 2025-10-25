/**
  *  \file test/game/vcr/classic/traditionalschedulertest.cpp
  *  \brief Test for game::vcr::classic::TraditionalScheduler
  */

#include "game/vcr/classic/traditionalscheduler.hpp"

#include <list>
#include <algorithm>

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "game/vcr/classic/scheduledeventconsumer.hpp"

using game::vcr::classic::EventListener;
using game::vcr::classic::ScheduledEvent;
using game::vcr::classic::Side;

namespace {
    bool matchEvent(const String_t& found, const String_t& expect)
    {
        String_t::size_type fpos = 0, epos = 0;
        while (fpos < found.size() && epos < expect.size()) {
            if (expect[epos] == '*') {
                fpos = found.find_first_of("(,)", fpos);
                ++epos;
            } else if (expect[epos] != found[fpos]) {
                return false;
            } else {
                ++fpos, ++epos;
            }
        }

        // Success if end reached for both
        return fpos >= found.size() && epos >= expect.size();
    }

    class TestScheduledEventConsumer : public game::vcr::classic::ScheduledEventConsumer {
     public:
        TestScheduledEventConsumer(afl::test::Assert a)
            : m_events(), m_assert(a)
            { }

        virtual void placeObject(Side side, const EventListener::UnitInfo& info)
            { m_events.push_back(afl::string::Format("placeObject(%d,%d)", int(side), info.position)); }
        virtual void pushEvent(ScheduledEvent e)
            { m_events.push_back(afl::string::Format("%s(%d,%d,%d,%d,%d,%d)") << ScheduledEvent::toString(e.type) << int(e.side) << e.a << e.b << e.c << e.d << e.e); }
        virtual void removeAnimations(int32_t from, int32_t to)
            { m_events.push_back(afl::string::Format("removeAnimations(%d,%d)", from, to)); }

        void assertEvent(String_t event)
            {
                m_assert(event).check("must have event", !m_events.empty());
                if (!matchEvent(m_events.front(), event)) {
                    m_assert.fail(afl::string::Format("event mismatch: found '%s', expected '%s'", m_events.front(), event));
                }
                m_events.pop_front();
            }

        void assertFinish()
            {
                m_assert.check("expect no more events", m_events.empty());
            }

     private:
        std::list<String_t> m_events;
        afl::test::Assert m_assert;
    };
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:placeObject", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::UnitInfo info;
    info.position = 7777;
    testee.placeObject(game::vcr::classic::RightSide, info);
    mock.assertEvent("placeObject(1,7777)");
    mock.assertFinish();
}

/* updateTime() is the baseline test.
   Since updateTime() serves as a "flush" operation, we also use it for all other operations. */
AFL_TEST("game.vcr.classic.TraditionalScheduler:updateTime", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateTime(500, 3000);
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:startFighter", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.startFighter(game::vcr::classic::RightSide, 5, 2000, 1000, -1);
    testee.updateTime(500, 3000);
    mock.assertEvent("StartFighter(1,5,2000,1000,*,*)");
    mock.assertEvent("UpdateNumFighters(1,-1,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:landFighter", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.landFighter(game::vcr::classic::RightSide, 5, +1);
    testee.updateTime(500, 3000);
    mock.assertEvent("RemoveFighter(1,5,*,*,*,*)");
    mock.assertEvent("UpdateNumFighters(1,1,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:killFighter", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.killFighter(game::vcr::classic::RightSide, 5);
    testee.updateTime(500, 3000);
    mock.assertEvent("ExplodeFighter(1,5,*,*,*,*)");
    mock.assertEvent("RemoveFighter(1,5,*,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:fireBeam:s/s", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::HitEffect eff;
    eff.damageDone = 2;
    eff.crewKilled = 3;
    eff.shieldLost = 4;

    testee.fireBeam(game::vcr::classic::RightSide, -10, -3, 50, 5, 6, eff);
    testee.fireBeam(game::vcr::classic::LeftSide,  -9, -3, 50, 5, 6, eff);
    testee.updateTime(500, 3000);
    // Beam/hit #1
    mock.assertEvent("FireBeamShipShip(1,9,*,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("HitObject(0,2,3,4,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    // Beam/hit #2
    mock.assertEvent("FireBeamShipShip(0,8,*,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("HitObject(1,2,3,4,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    // Tick
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:fireBeam:s/f", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::HitEffect eff;
    eff.damageDone = 2;
    eff.crewKilled = 3;
    eff.shieldLost = 4;

    testee.fireBeam(game::vcr::classic::RightSide, -8, 17, 50, 5, 6, eff);
    testee.fireBeam(game::vcr::classic::LeftSide,  -7, 12, 50, 5, 6, eff);
    testee.updateTime(500, 3000);
    mock.assertEvent("FireBeamShipFighter(1,17,7,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("FireBeamShipFighter(0,12,6,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:fireBeam:f/s", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::HitEffect eff;
    eff.damageDone = 2;
    eff.crewKilled = 3;
    eff.shieldLost = 4;

    testee.fireBeam(game::vcr::classic::RightSide, 12, -3, 50, 5, 6, eff);
    testee.updateTime(500, 3000);
    // Beam
    mock.assertEvent("FireBeamFighterShip(1,12,*,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    // Hit
    mock.assertEvent("HitObject(0,2,3,4,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    // Tick
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:fireBeam:f/f", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::HitEffect eff;
    eff.damageDone = 2;
    eff.crewKilled = 3;
    eff.shieldLost = 4;

    testee.fireBeam(game::vcr::classic::RightSide, 12, 9, 50, 5, 6, eff);
    testee.updateTime(500, 3000);

    mock.assertEvent("FireBeamFighterFighter(1,12,9,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:fireTorpedo", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    EventListener::HitEffect eff;
    eff.damageDone = 20;
    eff.crewKilled = 30;
    eff.shieldLost = 40;

    testee.fireTorpedo(game::vcr::classic::RightSide, 10, 5, -1, eff);
    testee.updateTime(500, 3000);

    mock.assertEvent("FireTorpedo(1,5,10,*,6,*)");
    mock.assertEvent("UpdateNumTorpedoes(1,-1,*,*,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");

    mock.assertEvent("HitObject(0,20,30,40,*,*)");
    mock.assertEvent("WaitAnimation(*,*,*,*,*,*)");

    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:updateBeam", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateBeam(game::vcr::classic::RightSide, 7, 500);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateBeam(1,7,500,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:updateLauncher", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateLauncher(game::vcr::classic::RightSide, 9, 200);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateLauncher(1,9,200,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:moveObject", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.moveObject(game::vcr::classic::LeftSide, 1000);
    testee.moveObject(game::vcr::classic::RightSide, 3000);
    testee.updateTime(500, 3000);

    mock.assertEvent("MoveObject(0,1000,*,*,*,*)");
    mock.assertEvent("MoveObject(1,3000,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:moveFighter", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.moveFighter(game::vcr::classic::RightSide, 7, 2000, 1000, game::vcr::classic::FighterReturns);
    testee.updateTime(500, 3000);

    mock.assertEvent("MoveFighter(1,7,2000,1000,2,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:killObject", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.killObject(game::vcr::classic::RightSide);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:updateObject", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateObject(game::vcr::classic::LeftSide, 50, 270, 5);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateObject(0,50,270,5,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:updateAmmo", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateAmmo(game::vcr::classic::RightSide, 77, 5);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateAmmo(1,77,5,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:updateFighter", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.updateFighter(game::vcr::classic::RightSide, 15, 2000, 1500, game::vcr::classic::FighterAttacks);
    testee.updateTime(500, 3000);

    mock.assertEvent("UpdateFighter(1,15,2000,1500,1,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:setResult", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.setResult(game::vcr::classic::BattleResult_t());
    testee.updateTime(500, 3000);

    mock.assertEvent("SetResult(*,*,*,*,*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}

AFL_TEST("game.vcr.classic.TraditionalScheduler:removeAnimations", a)
{
    TestScheduledEventConsumer mock(a);
    game::vcr::classic::TraditionalScheduler testee(mock);

    testee.removeAnimations();
    testee.updateTime(500, 3000);

    mock.assertEvent("removeAnimations(*,*)");
    mock.assertEvent("UpdateTime(*,500,*,*,*,*)");
    mock.assertEvent("UpdateDistance(*,3000,*,*,*,*)");
    mock.assertEvent("WaitTick(*,*,*,*,*,*)");
    mock.assertFinish();
}
