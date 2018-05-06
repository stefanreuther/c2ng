/**
  *  \file u/t_gfx_anim_sprite.cpp
  *  \brief Test for gfx::anim::Sprite
  */

#include "gfx/anim/sprite.hpp"

#include "t_gfx_anim.hpp"

/** Interface test. */
void
TestGfxAnimSprite::testInterface()
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

