/**
  *  \file test/gfx/timertest.cpp
  *  \brief Test for gfx::Timer
  */

#include "gfx/timer.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. This is an interface; just test instantiatability. */
AFL_TEST_NOARG("gfx.Timer")
{
    class Tester : public gfx::Timer {
     public:
        virtual void setInterval(afl::sys::Timeout_t)
            { }
    };
    Tester t;
    t.sig_fire.raise();
}
