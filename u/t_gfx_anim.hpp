/**
  *  \file u/t_gfx_anim.hpp
  *  \brief Tests for gfx::anim
  */
#ifndef C2NG_U_T_GFX_ANIM_HPP
#define C2NG_U_T_GFX_ANIM_HPP

#include <cxxtest/TestSuite.h>

class TestGfxAnimController : public CxxTest::TestSuite {
 public:
    void testFindRemove();
};

class TestGfxAnimSprite : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
