/**
  *  \file u/t_gfx_threed_vecmath.cpp
  *  \brief Test for gfx::threed::VecMath
  */

#include "gfx/threed/vecmath.hpp"

#include "t_gfx_threed.hpp"

using gfx::threed::Vec3f;
using gfx::threed::Vec4f;
using gfx::threed::Mat4f;

/*
 *  3D Vector
 */

void
TestGfxThreedVecMath::testVec3fMake()
{
    Vec3f v = Vec3f(4,5,6);
    TS_ASSERT_EQUALS(v(0), 4.0);
    TS_ASSERT_EQUALS(v(1), 5.0);
    TS_ASSERT_EQUALS(v(2), 6.0);
}

void
TestGfxThreedVecMath::testVec3fSub()
{
    Vec3f v = Vec3f(10,20,30) - Vec3f(3,5,7);
    TS_ASSERT_EQUALS(v(0),  7.0);
    TS_ASSERT_EQUALS(v(1), 15.0);
    TS_ASSERT_EQUALS(v(2), 23.0);
}

void
TestGfxThreedVecMath::testVec3fAdd()
{
    Vec3f v = Vec3f(10,20,30) + Vec3f(1,2,4);
    TS_ASSERT_EQUALS(v(0), 11.0);
    TS_ASSERT_EQUALS(v(1), 22.0);
    TS_ASSERT_EQUALS(v(2), 34.0);
}

void
TestGfxThreedVecMath::testVec3fLength()
{
    TS_ASSERT_EQUALS(Vec3f(3,4,0).length(), 5.0);
    TS_ASSERT_EQUALS(Vec3f(3,0,4).length(), 5.0);
    TS_ASSERT_EQUALS(Vec3f(0,3,4).length(), 5.0);
    TS_ASSERT_EQUALS(Vec3f(0,0,0).length(), 0.0);
}

void
TestGfxThreedVecMath::testVec3fNorm()
{
    Vec3f v = Vec3f(5,0,0).norm();
    TS_ASSERT_EQUALS(v(0), 1.0);
    TS_ASSERT_EQUALS(v(1), 0.0);
    TS_ASSERT_EQUALS(v(2), 0.0);

    v = Vec3f(3,0,4).norm();
    TS_ASSERT_EQUALS(v(0), float(3.0/5.0)); // float() cast required to match precision
    TS_ASSERT_EQUALS(v(1), 0.0);
    TS_ASSERT_EQUALS(v(2), float(4.0/5.0));
}

void
TestGfxThreedVecMath::testVec3fProd()
{
    Vec3f v = Vec3f(1,0,0).prod(Vec3f(0,1,0));
    TS_ASSERT_EQUALS(v(0), 0.0);
    TS_ASSERT_EQUALS(v(1), 0.0);
    TS_ASSERT_EQUALS(v(2), 1.0);

    v = Vec3f(3,4,5).prod(Vec3f(5,8,2));
    TS_ASSERT_EQUALS(v(0), -32.0);
    TS_ASSERT_EQUALS(v(1),  19.0);
    TS_ASSERT_EQUALS(v(2),   4.0);
}

void
TestGfxThreedVecMath::testVec3fScale()
{
    Vec3f v = Vec3f(10,15,20) * 3;
    TS_ASSERT_EQUALS(v(0), 30.0);
    TS_ASSERT_EQUALS(v(1), 45.0);
    TS_ASSERT_EQUALS(v(2), 60.0);
}

void
TestGfxThreedVecMath::testVec3fDot()
{
    // Perpendicular
    TS_ASSERT_EQUALS(Vec3f(5,0,0).dot(Vec3f(0,6,0)), 0);

    // Identical/Antiparallel
    TS_ASSERT_EQUALS(Vec3f(0,0,3).dot(Vec3f(0,0,3)), 9);
    TS_ASSERT_EQUALS(Vec3f(0,0,3).dot(Vec3f(0,0,-3)), -9);

    // Random
    TS_ASSERT_EQUALS(Vec3f(1,2,3).dot(Vec3f(4,5,6)), 32);
}

void
TestGfxThreedVecMath::testVec3fPer()
{
    Vec3f a(1,2,3);
    Vec3f v = a.per();
    TS_ASSERT(v.length() > 0);
    TS_ASSERT_EQUALS(a.dot(v), 0);

    a = Vec3f(4,4,4);
    v = a.per();
    TS_ASSERT(v.length() > 0);
    TS_ASSERT_EQUALS(a.dot(v), 0);
}

void
TestGfxThreedVecMath::testVec3fTransform()
{
    // Identity transform
    Vec3f v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                           0,1,0,0,
                                           0,0,1,0,
                                           0,0,0,1));
    TS_ASSERT_EQUALS(v(0), 7);
    TS_ASSERT_EQUALS(v(1), 8);
    TS_ASSERT_EQUALS(v(2), 9);

    // Translation
    v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     10,20,30,1));
    TS_ASSERT_EQUALS(v(0), 17);
    TS_ASSERT_EQUALS(v(1), 28);
    TS_ASSERT_EQUALS(v(2), 39);

    // Scaling
    v = Vec3f(7,8,9).transform(Mat4f(2,0,0,0,
                                     0,3,0,0,
                                     0,0,4,0,
                                     0,0,0,1));
    TS_ASSERT_EQUALS(v(0), 14);
    TS_ASSERT_EQUALS(v(1), 24);
    TS_ASSERT_EQUALS(v(2), 36);

    // Perspective
    v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     0,0,0,2));
    TS_ASSERT_EQUALS(v(0), 3.5);
    TS_ASSERT_EQUALS(v(1), 4.0);
    TS_ASSERT_EQUALS(v(2), 4.5);
}

/*
 *  4D Vector
 */

void
TestGfxThreedVecMath::testVec4fMake()
{
    Vec4f v = Vec4f(4,5,6,7);
    TS_ASSERT_EQUALS(v(0), 4.0);
    TS_ASSERT_EQUALS(v(1), 5.0);
    TS_ASSERT_EQUALS(v(2), 6.0);
    TS_ASSERT_EQUALS(v(3), 7.0);
}

/*
 *  Matrix
 */

void
TestGfxThreedVecMath::testMat4fMake()
{
    Mat4f m = Mat4f();
    TS_ASSERT_EQUALS(m(0), 0);
    TS_ASSERT_EQUALS(m(15), 0);

    m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    TS_ASSERT_EQUALS(m(0), 1);
    TS_ASSERT_EQUALS(m(15), 16);
}

void
TestGfxThreedVecMath::testMat4fMakeIdentity()
{
    Mat4f m = Mat4f::identity();
    TS_ASSERT_EQUALS(m(0), 1);
    TS_ASSERT_EQUALS(m(1), 0);
    TS_ASSERT_EQUALS(m(15), 1);

    Vec3f v = Vec3f(33,44,55).transform(m);
    TS_ASSERT_EQUALS(v(0), 33);
    TS_ASSERT_EQUALS(v(1), 44);
    TS_ASSERT_EQUALS(v(2), 55);
}

void
TestGfxThreedVecMath::testMat4fClone()
{
    // This test case is more meaningful in the JS version where we need to care about cloning.
    Mat4f m = Mat4f::identity();
    Mat4f n = m;
    n(0) = 7;
    TS_ASSERT_EQUALS(m(0), 1);
    TS_ASSERT_EQUALS(n(0), 7);
}

void
TestGfxThreedVecMath::testMat4fMakePerspectiveFinite()
{
    Mat4f m = Mat4f::perspective(2, 1.5, 3.0, 100.0);

    TS_ASSERT_DELTA(m(0), 0.42806, 0.00001);
    TS_ASSERT_EQUALS(m(1), 0);
    TS_ASSERT_EQUALS(m(2), 0);
    TS_ASSERT_EQUALS(m(3), 0);

    TS_ASSERT_EQUALS(m(4), 0);
    TS_ASSERT_DELTA(m(5), 0.64209, 0.00001);
    TS_ASSERT_EQUALS(m(6), 0);
    TS_ASSERT_EQUALS(m(7), 0);

    TS_ASSERT_EQUALS(m(8), 0);
    TS_ASSERT_EQUALS(m(9), 0);
    TS_ASSERT_DELTA(m(10), -1.061855, 0.000001);    // 103 / -97
    TS_ASSERT_EQUALS(m(11), -1);

    TS_ASSERT_EQUALS(m(12), 0);
    TS_ASSERT_EQUALS(m(13), 0);
    TS_ASSERT_DELTA(m(14), -6.185567, 0.000001);    // 300 / -97 * 2
    TS_ASSERT_EQUALS(m(15), 0);
}

void
TestGfxThreedVecMath::testMat4fMakePerspectiveInfinite()
{
    Mat4f m = Mat4f::perspective(2, 1.5, 3.0);

    TS_ASSERT_DELTA(m(0), 0.42806, 0.00001);
    TS_ASSERT_EQUALS(m(1), 0);
    TS_ASSERT_EQUALS(m(2), 0);
    TS_ASSERT_EQUALS(m(3), 0);

    TS_ASSERT_EQUALS(m(4), 0);
    TS_ASSERT_DELTA(m(5), 0.64209, 0.00001);
    TS_ASSERT_EQUALS(m(6), 0);
    TS_ASSERT_EQUALS(m(7), 0);

    TS_ASSERT_EQUALS(m(8), 0);
    TS_ASSERT_EQUALS(m(9), 0);
    TS_ASSERT_EQUALS(m(10), -1);
    TS_ASSERT_EQUALS(m(11), -1);

    TS_ASSERT_EQUALS(m(12), 0);
    TS_ASSERT_EQUALS(m(13), 0);
    TS_ASSERT_EQUALS(m(14), -6);
    TS_ASSERT_EQUALS(m(15), 0);
}

void
TestGfxThreedVecMath::testMat4fInvertSingular()
{
    Mat4f m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    bool n = m.invert();
    TS_ASSERT_EQUALS(n, false);
}

void
TestGfxThreedVecMath::testMat4fInvertIdentity()
{
    Mat4f m = Mat4f::identity();
    bool n = m.invert();
    TS_ASSERT_EQUALS(n, true);
    TS_ASSERT_EQUALS(m(0),  1);  TS_ASSERT_EQUALS(m(1),  0);  TS_ASSERT_EQUALS(m(2),  0);  TS_ASSERT_EQUALS(m(3),  0);
    TS_ASSERT_EQUALS(m(4),  0);  TS_ASSERT_EQUALS(m(5),  1);  TS_ASSERT_EQUALS(m(6),  0);  TS_ASSERT_EQUALS(m(7),  0);
    TS_ASSERT_EQUALS(m(8),  0);  TS_ASSERT_EQUALS(m(9),  0);  TS_ASSERT_EQUALS(m(10), 1);  TS_ASSERT_EQUALS(m(11), 0);
    TS_ASSERT_EQUALS(m(12), 0);  TS_ASSERT_EQUALS(m(13), 0);  TS_ASSERT_EQUALS(m(14), 0);  TS_ASSERT_EQUALS(m(15), 1);
}

void
TestGfxThreedVecMath::testMat4fInvertMisc()
{
    // Reference result obtained with Wolfram Alpha
    Mat4f m = Mat4f(1,2,3,4, 1,9,8,7, 5,60,7,80, 9,10,11,12);
    bool n = m.invert();
    TS_ASSERT_EQUALS(n, true);
    TS_ASSERT_DELTA(m(0), -133 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(1),  -56 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(2),   0 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(3),   77 / 504.0, 0.00001);
    TS_ASSERT_DELTA(m(4), -246 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(5),   64 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(6),   4 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(7),   18 / 504.0, 0.00001);
    TS_ASSERT_DELTA(m(8),  135 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(9),   40 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(10), -8 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(11), -15 / 504.0, 0.00001);
    TS_ASSERT_DELTA(m(12), 181 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(13), -48 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(14),  4 / 504.0, 0.00001);  TS_ASSERT_DELTA(m(15), -17 / 504.0, 0.00001);
}

void
TestGfxThreedVecMath::testMat4fTranspose()
{
    Mat4f m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    Mat4f& n = m.transpose();
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_EQUALS(m(0),  1);  TS_ASSERT_EQUALS(m(1),  5);  TS_ASSERT_EQUALS(m(2),   9);  TS_ASSERT_EQUALS(m(3),  13);
    TS_ASSERT_EQUALS(m(4),  2);  TS_ASSERT_EQUALS(m(5),  6);  TS_ASSERT_EQUALS(m(6),  10);  TS_ASSERT_EQUALS(m(7),  14);
    TS_ASSERT_EQUALS(m(8),  3);  TS_ASSERT_EQUALS(m(9),  7);  TS_ASSERT_EQUALS(m(10), 11);  TS_ASSERT_EQUALS(m(11), 15);
    TS_ASSERT_EQUALS(m(12), 4);  TS_ASSERT_EQUALS(m(13), 8);  TS_ASSERT_EQUALS(m(14), 12);  TS_ASSERT_EQUALS(m(15), 16);
}

void
TestGfxThreedVecMath::testMat4fTranslate()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.translate(Vec3f(3,4,5));
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_EQUALS(m(0),  1);  TS_ASSERT_EQUALS(m(1),  0);  TS_ASSERT_EQUALS(m(2),  0);  TS_ASSERT_EQUALS(m(3),  0);
    TS_ASSERT_EQUALS(m(4),  0);  TS_ASSERT_EQUALS(m(5),  1);  TS_ASSERT_EQUALS(m(6),  0);  TS_ASSERT_EQUALS(m(7),  0);
    TS_ASSERT_EQUALS(m(8),  0);  TS_ASSERT_EQUALS(m(9),  0);  TS_ASSERT_EQUALS(m(10), 1);  TS_ASSERT_EQUALS(m(11), 0);
    TS_ASSERT_EQUALS(m(12), 3);  TS_ASSERT_EQUALS(m(13), 4);  TS_ASSERT_EQUALS(m(14), 5);  TS_ASSERT_EQUALS(m(15), 1);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_EQUALS(v(0), 13);
    TS_ASSERT_EQUALS(v(1), 24);
    TS_ASSERT_EQUALS(v(2), 35);
}

void
TestGfxThreedVecMath::testMat4fScaleVector()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.scale(Vec3f(3,4,5));
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_EQUALS(m(0),  3);  TS_ASSERT_EQUALS(m(1),  0);  TS_ASSERT_EQUALS(m(2),  0);  TS_ASSERT_EQUALS(m(3),  0);
    TS_ASSERT_EQUALS(m(4),  0);  TS_ASSERT_EQUALS(m(5),  4);  TS_ASSERT_EQUALS(m(6),  0);  TS_ASSERT_EQUALS(m(7),  0);
    TS_ASSERT_EQUALS(m(8),  0);  TS_ASSERT_EQUALS(m(9),  0);  TS_ASSERT_EQUALS(m(10), 5);  TS_ASSERT_EQUALS(m(11), 0);
    TS_ASSERT_EQUALS(m(12), 0);  TS_ASSERT_EQUALS(m(13), 0);  TS_ASSERT_EQUALS(m(14), 0);  TS_ASSERT_EQUALS(m(15), 1);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_EQUALS(v(0), 30);
    TS_ASSERT_EQUALS(v(1), 80);
    TS_ASSERT_EQUALS(v(2), 150);
}

void
TestGfxThreedVecMath::testMat4fScaleScalar()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.scale(6);
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_EQUALS(m(0),  6);  TS_ASSERT_EQUALS(m(1),  0);  TS_ASSERT_EQUALS(m(2),  0);  TS_ASSERT_EQUALS(m(3),  0);
    TS_ASSERT_EQUALS(m(4),  0);  TS_ASSERT_EQUALS(m(5),  6);  TS_ASSERT_EQUALS(m(6),  0);  TS_ASSERT_EQUALS(m(7),  0);
    TS_ASSERT_EQUALS(m(8),  0);  TS_ASSERT_EQUALS(m(9),  0);  TS_ASSERT_EQUALS(m(10), 6);  TS_ASSERT_EQUALS(m(11), 0);
    TS_ASSERT_EQUALS(m(12), 0);  TS_ASSERT_EQUALS(m(13), 0);  TS_ASSERT_EQUALS(m(14), 0);  TS_ASSERT_EQUALS(m(15), 1);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_EQUALS(v(0), 60);
    TS_ASSERT_EQUALS(v(1), 120);
    TS_ASSERT_EQUALS(v(2), 180);
}

void
TestGfxThreedVecMath::testMat4fRotateX()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateX(M_PI/2);
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_DELTA(m(0),  1, 0.000001);  TS_ASSERT_DELTA(m(1),  0, 0.000001);  TS_ASSERT_DELTA(m(2),  0, 0.000001);  TS_ASSERT_DELTA(m(3),  0, 0.000001);
    TS_ASSERT_DELTA(m(4),  0, 0.000001);  TS_ASSERT_DELTA(m(5),  0, 0.000001);  TS_ASSERT_DELTA(m(6),  1, 0.000001);  TS_ASSERT_DELTA(m(7),  0, 0.000001);
    TS_ASSERT_DELTA(m(8),  0, 0.000001);  TS_ASSERT_DELTA(m(9), -1, 0.000001);  TS_ASSERT_DELTA(m(10), 0, 0.000001);  TS_ASSERT_DELTA(m(11), 0, 0.000001);
    TS_ASSERT_DELTA(m(12), 0, 0.000001);  TS_ASSERT_DELTA(m(13), 0, 0.000001);  TS_ASSERT_DELTA(m(14), 0, 0.000001);  TS_ASSERT_DELTA(m(15), 1, 0.000001);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_DELTA(v(0),  10, 0.000001);
    TS_ASSERT_DELTA(v(1), -30, 0.000001);
    TS_ASSERT_DELTA(v(2),  20, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fRotateY()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateY(M_PI/2);
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_DELTA(m(0),  0, 0.000001);  TS_ASSERT_DELTA(m(1),  0, 0.000001);  TS_ASSERT_DELTA(m(2), -1, 0.000001);  TS_ASSERT_DELTA(m(3),  0, 0.000001);
    TS_ASSERT_DELTA(m(4),  0, 0.000001);  TS_ASSERT_DELTA(m(5),  1, 0.000001);  TS_ASSERT_DELTA(m(6),  0, 0.000001);  TS_ASSERT_DELTA(m(7),  0, 0.000001);
    TS_ASSERT_DELTA(m(8),  1, 0.000001);  TS_ASSERT_DELTA(m(9),  0, 0.000001);  TS_ASSERT_DELTA(m(10), 0, 0.000001);  TS_ASSERT_DELTA(m(11), 0, 0.000001);
    TS_ASSERT_DELTA(m(12), 0, 0.000001);  TS_ASSERT_DELTA(m(13), 0, 0.000001);  TS_ASSERT_DELTA(m(14), 0, 0.000001);  TS_ASSERT_DELTA(m(15), 1, 0.000001);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_DELTA(v(0),  30, 0.000001);
    TS_ASSERT_DELTA(v(1),  20, 0.000001);
    TS_ASSERT_DELTA(v(2), -10, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fRotateZ()
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateZ(M_PI/2);
    TS_ASSERT_EQUALS(&m, &n);
    TS_ASSERT_DELTA(m(0),  0, 0.000001);  TS_ASSERT_DELTA(m(1),  1, 0.000001);  TS_ASSERT_DELTA(m(2),  0, 0.000001);  TS_ASSERT_DELTA(m(3),  0, 0.000001);
    TS_ASSERT_DELTA(m(4), -1, 0.000001);  TS_ASSERT_DELTA(m(5),  0, 0.000001);  TS_ASSERT_DELTA(m(6),  0, 0.000001);  TS_ASSERT_DELTA(m(7),  0, 0.000001);
    TS_ASSERT_DELTA(m(8),  0, 0.000001);  TS_ASSERT_DELTA(m(9),  0, 0.000001);  TS_ASSERT_DELTA(m(10), 1, 0.000001);  TS_ASSERT_DELTA(m(11), 0, 0.000001);
    TS_ASSERT_DELTA(m(12), 0, 0.000001);  TS_ASSERT_DELTA(m(13), 0, 0.000001);  TS_ASSERT_DELTA(m(14), 0, 0.000001);  TS_ASSERT_DELTA(m(15), 1, 0.000001);

    Vec3f v = Vec3f(10,20,30).transform(m);
    TS_ASSERT_DELTA(v(0), -20, 0.000001);
    TS_ASSERT_DELTA(v(1),  10, 0.000001);
    TS_ASSERT_DELTA(v(2),  30, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fMultiplyInPlace()
{
    // Reference result obtained with Wolfram Alpha
    Mat4f m(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    Mat4f n(20,19,18,17, 16,15,14,13, 12,11,10,9, 8,7,6,5);
    Mat4f& prod = (m *= n);
    TS_ASSERT_EQUALS(&m, &prod);
    TS_ASSERT_EQUALS(m(0),  498);  TS_ASSERT_EQUALS(m(1),  572);  TS_ASSERT_EQUALS(m(2),  646);  TS_ASSERT_EQUALS(m(3),  720);
    TS_ASSERT_EQUALS(m(4),  386);  TS_ASSERT_EQUALS(m(5),  444);  TS_ASSERT_EQUALS(m(6),  502);  TS_ASSERT_EQUALS(m(7),  560);
    TS_ASSERT_EQUALS(m(8),  274);  TS_ASSERT_EQUALS(m(9),  316);  TS_ASSERT_EQUALS(m(10), 358);  TS_ASSERT_EQUALS(m(11), 400);
    TS_ASSERT_EQUALS(m(12), 162);  TS_ASSERT_EQUALS(m(13), 188);  TS_ASSERT_EQUALS(m(14), 214);  TS_ASSERT_EQUALS(m(15), 240);
}

void
TestGfxThreedVecMath::testMat4fMultiply()
{
    const Mat4f m(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    const Mat4f n(20,19,18,17, 16,15,14,13, 12,11,10,9, 8,7,6,5);
    const Mat4f prod = (m * n);
    TS_ASSERT_EQUALS(prod(0),  498);  TS_ASSERT_EQUALS(prod(1),  572);  TS_ASSERT_EQUALS(prod(2),  646);  TS_ASSERT_EQUALS(prod(3),  720);
    TS_ASSERT_EQUALS(prod(4),  386);  TS_ASSERT_EQUALS(prod(5),  444);  TS_ASSERT_EQUALS(prod(6),  502);  TS_ASSERT_EQUALS(prod(7),  560);
    TS_ASSERT_EQUALS(prod(8),  274);  TS_ASSERT_EQUALS(prod(9),  316);  TS_ASSERT_EQUALS(prod(10), 358);  TS_ASSERT_EQUALS(prod(11), 400);
    TS_ASSERT_EQUALS(prod(12), 162);  TS_ASSERT_EQUALS(prod(13), 188);  TS_ASSERT_EQUALS(prod(14), 214);  TS_ASSERT_EQUALS(prod(15), 240);
}

void
TestGfxThreedVecMath::testMat4fTransformMoveScaleRotate()
{
    Mat4f m = Mat4f::identity();
    m.rotateZ(M_PI/2);                // third operation
    m.scale(2);                       // second operation
    m.translate(Vec3f(50,40,30));     // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    TS_ASSERT_DELTA(v(0), -92, 0.000001);
    TS_ASSERT_DELTA(v(1), 110, 0.000001);
    TS_ASSERT_DELTA(v(2),  74, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fTransformMoveScale()
{
    Mat4f m = Mat4f::identity();
    m.scale(2);                       // second operation
    m.translate(Vec3f(50,40,30));     // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    TS_ASSERT_DELTA(v(0), 110, 0.000001);
    TS_ASSERT_DELTA(v(1),  92, 0.000001);
    TS_ASSERT_DELTA(v(2),  74, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fTransformScaleMove()
{
    Mat4f m = Mat4f::identity();
    m.translate(Vec3f(50,40,30));     // second operation
    m.scale(2);                       // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    TS_ASSERT_DELTA(v(0), 60, 0.000001);
    TS_ASSERT_DELTA(v(1), 52, 0.000001);
    TS_ASSERT_DELTA(v(2), 44, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fTransformRotateX()
{
    Mat4f m = Mat4f::identity();
    m.rotateX(M_PI/4);

    Vec3f v = Vec3f(44,1,0).transform(m);
    TS_ASSERT_DELTA(v(0), 44, 0.000001);
    TS_ASSERT_DELTA(v(1), 0.707106, 0.000001);
    TS_ASSERT_DELTA(v(2), 0.707106, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fTransformRotateY()
{
    Mat4f m = Mat4f::identity();
    m.rotateY(M_PI/4);
    Vec3f v = Vec3f(1,44,0).transform(m);
    TS_ASSERT_DELTA(v(0), 0.707106, 0.000001);
    TS_ASSERT_DELTA(v(1), 44, 0.000001);
    TS_ASSERT_DELTA(v(2), -0.707106, 0.000001);
}

void
TestGfxThreedVecMath::testMat4fTransformRotateZ()
{
    Mat4f m = Mat4f::identity();
    m.rotateZ(M_PI/4);
    Vec3f v = Vec3f(1,0,44).transform(m);
    TS_ASSERT_DELTA(v(0), 0.707106, 0.000001);
    TS_ASSERT_DELTA(v(1), 0.707106, 0.000001);
    TS_ASSERT_DELTA(v(2), 44, 0.000001);
}

