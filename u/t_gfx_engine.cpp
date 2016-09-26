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
        virtual afl::base::Ptr<gfx::Canvas> createWindow(int /*width*/, int /*height*/, int /*bpp*/, WindowFlags_t /*flags*/)
            { return 0; }
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& /*file*/)
            { return 0; }
        virtual void handleEvent(gfx::EventConsumer& /*consumer*/, bool /*relativeMouseMovement*/)
            { }
        virtual util::RequestDispatcher& dispatcher()
            { return *this; }
        virtual afl::base::Ptr<gfx::Timer> createTimer()
            { return 0; }
     private:
        virtual void postNewRunnable(afl::base::Runnable* /*p*/)
            { }
    };
    MyEngine testee;
}

