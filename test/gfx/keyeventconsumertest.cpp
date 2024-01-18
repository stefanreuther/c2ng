/**
  *  \file test/gfx/keyeventconsumertest.cpp
  *  \brief Test for gfx::KeyEventConsumer
  */

#include "gfx/keyeventconsumer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.KeyEventConsumer")
{
    class Tester : public gfx::KeyEventConsumer {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
    };
    Tester t;
}
