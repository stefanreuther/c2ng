/**
  *  \file u/t_gfx_colorscheme.cpp
  *  \brief Test for gfx::ColorScheme
  */

#include "gfx/colorscheme.hpp"

#include "t_gfx.hpp"

/** Interface test. */
void
TestGfxColorScheme::testInterface()
{
    class Tester : public gfx::ColorScheme<int> {
     public:
        virtual gfx::Color_t getColor(int /*index*/)
            { return gfx::Color_t(); }
        virtual void drawBackground(gfx::Canvas& /*can*/, const gfx::Rectangle& /*area*/)
            { }
    };
    Tester t;
}

