/**
  *  \file test/gfx/anim/controllertest.cpp
  *  \brief Test for gfx::anim::Controller
  */

#include "gfx/anim/controller.hpp"
#include "afl/test/testrunner.hpp"

/** Test find/delete loop.
    Must not get stuck. */
AFL_TEST("gfx.anim.Controller:find-remove", a)
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
    NullSprite* sa = new NullSprite(numLiveSprites);
    sa->setId(97);
    testee.addNewSprite(sa);

    // Add another sprite
    NullSprite* sb = new NullSprite(numLiveSprites);
    sb->setId(97);
    testee.addNewSprite(sb);

    // Must now have two live sprites
    a.checkEqual("01. numLiveSprites", numLiveSprites, 2);

    // Remove them both
    int n = 0;
    while (gfx::anim::Sprite* p = testee.findSpriteById(97)) {
        p->markForDeletion();
        ++n;
    }

    // Must have deleted both, but they are still alive
    a.checkEqual("11. num deleted", n, 2);
    a.checkEqual("12. numLiveSprites", numLiveSprites, 2);

    // tick() will kill them
    testee.tick();
    a.checkEqual("21. numLiveSprites", numLiveSprites, 0);
}
