/**
  *  \file test/gfx/timerqueuetest.cpp
  *  \brief Test for gfx::TimerQueue
  */

#include "gfx/timerqueue.hpp"

#include "afl/test/testrunner.hpp"
#include <string>

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
AFL_TEST("gfx.TimerQueue:sequence", a)
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
    a.checkEqual("01. getNextTimeout", testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);
    a.checkEqual("02. acc", acc, "");

    // Start two timers
    t1->setInterval(100);
    t2->setInterval(200);
    a.checkEqual("11. getNextTimeout", testee.getNextTimeout(), 100U);
    a.checkEqual("12. acc", acc, "");

    testee.handleElapsedTime(60);
    a.checkEqual("21. getNextTimeout", testee.getNextTimeout(), 40U);
    a.checkEqual("22. acc", acc, "");

    testee.handleElapsedTime(60);
    a.checkEqual("31. getNextTimeout", testee.getNextTimeout(), 80U);
    a.checkEqual("32. acc", acc, "1");

    testee.handleElapsedTime(80);
    a.checkEqual("41. getNextTimeout", testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);
    a.checkEqual("42. acc", acc, "12");

    // Destroy one timer
    t1 = 0;
}

/** Test that a timer outlives the TimerQueue. */
AFL_TEST_NOARG("gfx.TimerQueue:lifetime")
{
    afl::base::Ptr<gfx::Timer> t1;
    {
        gfx::TimerQueue testee;
        t1 = testee.createTimer().asPtr();
    }
}

/** Test that a timer dies while active. */
AFL_TEST("gfx.TimerQueue:dies-while-active", a)
{
    gfx::TimerQueue testee;
    afl::base::Ptr<gfx::Timer> t1 = testee.createTimer().asPtr();
    afl::base::Ptr<gfx::Timer> t2 = testee.createTimer().asPtr();

    // No timer has been set yet, so no timeout yet
    a.checkEqual("01. getNextTimeout", testee.getNextTimeout(), afl::sys::INFINITE_TIMEOUT);

    // Start two timers
    t1->setInterval(100);
    t2->setInterval(200);
    a.checkEqual("11. getNextTimeout", testee.getNextTimeout(), 100U);

    // Destroy timer 1. Next timeout changes to 200.
    t1 = 0;
    a.checkEqual("21. getNextTimeout", testee.getNextTimeout(), 200U);
}
