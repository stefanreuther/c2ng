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
        virtual afl::base::Ref<gfx::Canvas> createWindow(int /*width*/, int /*height*/, int /*bpp*/, WindowFlags_t /*flags*/)
            { throw "nix"; }
        virtual afl::base::Ref<gfx::Canvas> loadImage(afl::io::Stream& /*file*/)
            { throw "nix"; }
        virtual void handleEvent(gfx::EventConsumer& /*consumer*/, bool /*relativeMouseMovement*/)
            { }
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

