/**
  *  \file test/gfx/enginetest.cpp
  *  \brief Test for gfx::Engine
  */

#include "gfx/engine.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/canvas.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.Engine")
{
    class MyEngine : public gfx::Engine, private util::RequestDispatcher {
     public:
        virtual afl::base::Ref<gfx::Canvas> createWindow(const gfx::WindowParameters& /*param*/)
            { throw "nix"; }
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& /*file*/)
            { throw "nix"; }
        virtual void handleEvent(gfx::EventConsumer& /*consumer*/, bool /*relativeMouseMovement*/)
            { }
        virtual util::Key_t getKeyboardModifierState()
            { return 0; }
        virtual util::RequestDispatcher& dispatcher()
            { return *this; }
        virtual afl::base::Ref<gfx::Timer> createTimer()
            { throw "nix"; }
     private:
        virtual void postNewRunnable(afl::base::Runnable* /*p*/)
            { }
    };
    MyEngine testee;
}
