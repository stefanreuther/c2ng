/**
  *  \file u/t_gfx_anim_controller.cpp
  *  \brief Test for gfx::anim::Controller
  */

#include "gfx/anim/controller.hpp"

#include "t_gfx_anim.hpp"

/** Test find/delete loop.
    Must not get stuck. */
void
TestGfxAnimController::testFindRemove()
{
    class NullSprite : public gfx::anim::Sprite {
     public:
        NullSprite(int& num)
            : m_num(num)
            { ++m_num; }
        ~NullSprite()
            { --m_num; }
        virtual void draw(gfx::Canvas& /*can*/)
            { }
        virtual void tick()
            { }
     private:
        int& m_num;
    };
    gfx::anim::Controller testee;
    int numLiveSprites = 0;

    // Add a sprite
    NullSprite* a = new NullSprite(numLiveSprites);
    a->setId(97);
    testee.addNewSprite(a);

    // Add another sprite
    NullSprite* b = new NullSprite(numLiveSprites);
    b->setId(97);
    testee.addNewSprite(b);

    // Must now have two live sprites
    TS_ASSERT_EQUALS(numLiveSprites, 2);

    // Remove them both
    int n = 0;
    while (gfx::anim::Sprite* p = testee.findSpriteById(97)) {
        p->markForDeletion();
        ++n;
    }

    // Must have deleted both, but they are still alive
    TS_ASSERT_EQUALS(n, 2);
    TS_ASSERT_EQUALS(numLiveSprites, 2);

    // tick() will kill them
    testee.tick();
    TS_ASSERT_EQUALS(numLiveSprites, 0);
}

