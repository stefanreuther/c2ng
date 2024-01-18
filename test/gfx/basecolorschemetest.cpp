/**
  *  \file test/gfx/basecolorschemetest.cpp
  *  \brief Test for gfx::BaseColorScheme
  */

#include "gfx/basecolorscheme.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.BaseColorScheme")
{
    class Tester : public gfx::BaseColorScheme {
     public:
        virtual void drawBackground(gfx::Canvas& /*can*/, const gfx::Rectangle& /*area*/)
            { }
    };
    Tester t;
}
