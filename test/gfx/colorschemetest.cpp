/**
  *  \file test/gfx/colorschemetest.cpp
  *  \brief Test for gfx::ColorScheme
  */

#include "gfx/colorscheme.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.ColorScheme")
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
