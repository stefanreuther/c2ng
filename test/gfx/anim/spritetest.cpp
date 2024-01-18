/**
  *  \file test/gfx/anim/spritetest.cpp
  *  \brief Test for gfx::anim::Sprite
  */

#include "gfx/anim/sprite.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.anim.Sprite")
{
    class Tester : public gfx::anim::Sprite {
     public:
        virtual void draw(gfx::Canvas& /*can*/)
            { }
        virtual void tick()
            { }
    };
    Tester t;
}
