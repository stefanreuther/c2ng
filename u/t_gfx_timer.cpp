/**
  *  \file u/t_gfx_timer.cpp
  *  \brief Test for gfx::Timer
  */

#include "gfx/timer.hpp"

#include "t_gfx.hpp"

/** Simple test. This is an interface; just test instantiatability. */
void
TestGfxTimer::testIt()
{
    class Tester : public gfx::Timer {
     public:
        virtual void setInterval(afl::sys::Timeout_t)
            { }
    };
    Tester t;
    t.sig_fire.raise();
}
