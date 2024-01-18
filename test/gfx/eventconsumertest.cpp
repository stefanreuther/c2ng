/**
  *  \file test/gfx/eventconsumertest.cpp
  *  \brief Test for gfx::EventConsumer
  */

#include "gfx/eventconsumer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.EventConsumer")
{
    class Tester : public gfx::EventConsumer {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
    };
    Tester t;
}
