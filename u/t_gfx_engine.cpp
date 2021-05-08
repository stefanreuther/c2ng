/**
  *  \file u/t_gfx_engine.cpp
  *  \brief Test for gfx::Engine
  */

#include "gfx/engine.hpp"

#include "t_gfx.hpp"
#include "gfx/canvas.hpp"

/** Interface test. */
void
TestGfxEngine::testIt()
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

