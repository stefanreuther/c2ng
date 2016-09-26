/**
  *  \file u/t_gfx_eventconsumer.cpp
  *  \brief Test for gfx::EventConsumer
  */

#include "gfx/eventconsumer.hpp"

#include "t_gfx.hpp"

/** Interface test. */
void
TestGfxEventConsumer::testIt()
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

