/**
  *  \file test/client/map/overlaytest.cpp
  *  \brief Test for client::map::Overlay
  */

#include "client/map/overlay.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST("client.map.Overlay", a)
{
    class Tester : public client::map::Overlay {
     public:
        virtual void drawBefore(gfx::Canvas& /*can*/, const client::map::Renderer& /*ren*/)
            { }
        virtual void drawAfter(gfx::Canvas& /*can*/, const client::map::Renderer& /*ren*/)
            { }
        virtual bool drawCursor(gfx::Canvas& /*can*/, const client::map::Renderer& /*ren*/)
            { return false; }
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/, const client::map::Renderer& /*ren*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const client::map::Renderer& /*ren*/)
            { return false; }
    };
    Tester t;

    a.checkNull("getCallback", t.getCallback());
}
