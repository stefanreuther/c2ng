/**
  *  \file u/t_gfx_timerqueue.cpp
  *  \brief Test for gfx::TimerQueue
  */

#include <string>
#include "gfx/timerqueue.hpp"

#include "t_gfx.hpp"

namespace {
    class Handler {
     public:
        Handler(std::string& acc, std::string text)
            : m_acc(acc),
              m_text(text)
            { }
        void tick()
            { m_acc += m_text; }

     private:
        std::string& m_acc;
        std::string m_text;
    };
}

/** Basic functionality test. */
void
TestGfxTimerQueue::test1()
{
    std::string acc;
    Handler h1(acc, "1");
    Handler h2(acc, "2");

    // Set up
    gfx::TimerQueue testee;
    afl::base::Ptr<gfx::Timer> t1 = testee.createTimer().asPtr();
    afl::base::Ptr<gfx::Timer> t2 = testee.createTimer().asPtr();
    t1->sig_fire.add(&h1, &Handler::tick);
    t2->sig_fire.add(&h2, &Handler::tick);

    // No timer has been set yet, so no timeout yet
    TS_ASSERT_EQUALS(testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);
    TS_ASSERT_EQUALS(acc, "");

    // Start two timers
    t1->setInterval(100);
    t2->setInterval(200);
    TS_ASSERT_EQUALS(testee.getNextTimeout(), 100U);
    TS_ASSERT_EQUALS(acc, "");

    testee.handleElapsedTime(60);
    TS_ASSERT_EQUALS(testee.getNextTimeout(), 40U);
    TS_ASSERT_EQUALS(acc, "");

    testee.handleElapsedTime(60);
    TS_ASSERT_EQUALS(testee.getNextTimeout(), 80U);
    TS_ASSERT_EQUALS(acc, "1");

    testee.handleElapsedTime(80);
    TS_ASSERT_EQUALS(testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);
    TS_ASSERT_EQUALS(acc, "12");

    // Destroy one timer
    t1 = 0;
}

/** Test that a timer outlives the TimerQueue. */
void
TestGfxTimerQueue::test2()
{
    afl::base::Ptr<gfx::Timer> t1;
    {
        gfx::TimerQueue testee;
        t1 = testee.createTimer().asPtr();
    }
}

/** Test that a timer dies while active. */
void
TestGfxTimerQueue::test3()
{
    gfx::TimerQueue testee;
    afl::base::Ptr<gfx::Timer> t1 = testee.createTimer().asPtr();
    afl::base::Ptr<gfx::Timer> t2 = testee.createTimer().asPtr();

    // No timer has been set yet, so no timeout yet
    TS_ASSERT_EQUALS(testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);

    // Start two timers
    t1->setInterval(100);
    t2->setInterval(200);
    TS_ASSERT_EQUALS(testee.getNextTimeout(), 100U);

    // Destroy timer 1. Next timeout changes to 200.
    t1 = 0;
    TS_ASSERT_EQUALS(testee.getNextTimeout(), 200U);
}

