/**
  *  \file u/t_gfx_threed.hpp
  *  \brief Tests for gfx::threed
  */
#ifndef C2NG_U_T_GFX_THREED_HPP
#define C2NG_U_T_GFX_THREED_HPP

#include <cxxtest/TestSuite.h>

class TestGfxThreedContext : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGfxThreedModel : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testLoad();
    void testLoadTruncate();
    void testLoadErrors();
    void testUnsupportedBlock();
};

class TestGfxThreedPositionList : public CxxTest::TestSuite {
 public:
    void testInit();
    void testAccess();
    void testFindPointsTwoRanges();
    void testFindPointsOneRange();
    void testFindPointsSingleRange();
    void testFindPointsFixedOnlyOdd();
    void testFindPointsFixedOnlyEven();
    void testFindPointsEmpty();
    void testFindPointsOne();
};

class TestGfxThreedVecMath : public CxxTest::TestSuite {
 public:
    void testVec3fMake();
    void testVec3fSub();
    void testVec3fAdd();
    void testVec3fLength();
    void testVec3fNorm();
    void testVec3fProd();
    void testVec3fScale();
    void testVec3fDot();
    void testVec3fPer();
    void testVec3fTransform();
    void testVec4fMake();
    void testMat4fMake();
    void testMat4fMakeIdentity();
    void testMat4fClone();
    void testMat4fMakePerspectiveFinite();
    void testMat4fMakePerspectiveInfinite();
    void testMat4fInvertSingular();
    void testMat4fInvertIdentity();
    void testMat4fInvertMisc();
    void testMat4fTranspose();
    void testMat4fTranslate();
    void testMat4fScaleVector();
    void testMat4fScaleScalar();
    void testMat4fRotateX();
    void testMat4fRotateY();
    void testMat4fRotateZ();
    void testMat4fMultiplyInPlace();
    void testMat4fMultiply();
    void testMat4fTransformMoveScaleRotate();
    void testMat4fTransformMoveScale();
    void testMat4fTransformScaleMove();
    void testMat4fTransformRotateX();
    void testMat4fTransformRotateY();
    void testMat4fTransformRotateZ();
};

#endif
