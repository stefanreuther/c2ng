/**
  *  \file u/t_gfx_basecolorscheme.cpp
  *  \brief Test for gfx::BaseColorScheme
  */

#include "gfx/basecolorscheme.hpp"

#include "t_gfx.hpp"

/** Interface test. */
void
TestGfxBaseColorScheme::testIt()
{
    class Tester : public gfx::BaseColorScheme {
     public:
        virtual void drawBackground(gfx::Canvas& /*can*/, const gfx::Rectangle& /*area*/)
            { }
    };
    Tester t;
}

