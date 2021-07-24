/**
  *  \file u/t_gfx_keyeventconsumer.cpp
  *  \brief Test for gfx::KeyEventConsumer
  */

#include "gfx/keyeventconsumer.hpp"

#include "t_gfx.hpp"

/** Interface test. */
void
TestGfxKeyEventConsumer::testInterface()
{
    class Tester : public gfx::KeyEventConsumer {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
    };
    Tester t;
}

