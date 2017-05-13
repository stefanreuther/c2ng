/**
  *  \file u/t_gfx_gen.hpp
  *  \brief Tests for gfx::gen
  */
#ifndef C2NG_U_T_GFX_GEN_HPP
#define C2NG_U_T_GFX_GEN_HPP

#include <cxxtest/TestSuite.h>

class TestGfxGenPerlinNoise : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxGenSpaceView : public CxxTest::TestSuite {
 public:
    void testStarfieldRegression();
    void testStarRegression();
    void testNebulaRegression();
    void testSunRegression();
};

class TestGfxGenSpaceViewConfig : public CxxTest::TestSuite {
 public:
    void testRegression();
    void testRegressionDefault();
};

#endif
