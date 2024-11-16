/**
  *  \file test/gfx/threed/vecmathtest.cpp
  *  \brief Test for gfx::threed::VecMath
  */

#include "gfx/threed/vecmath.hpp"

#include "afl/test/testrunner.hpp"
#include "util/math.hpp"

using gfx::threed::Vec3f;
using gfx::threed::Vec4f;
using gfx::threed::Mat4f;

/*
 *  3D Vector
 */

AFL_TEST("gfx.threed.VecMath:Vec3f:make", a)
{
    Vec3f v = Vec3f(4,5,6);
    a.checkEqual("01", v(0), 4.0);
    a.checkEqual("02", v(1), 5.0);
    a.checkEqual("03", v(2), 6.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:sub", a)
{
    Vec3f v = Vec3f(10,20,30) - Vec3f(3,5,7);
    a.checkEqual("01", v(0),  7.0);
    a.checkEqual("02", v(1), 15.0);
    a.checkEqual("03", v(2), 23.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:add", a)
{
    Vec3f v = Vec3f(10,20,30) + Vec3f(1,2,4);
    a.checkEqual("01", v(0), 11.0);
    a.checkEqual("02", v(1), 22.0);
    a.checkEqual("03", v(2), 34.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:length", a)
{
    a.checkEqual("01", Vec3f(3,4,0).length(), 5.0);
    a.checkEqual("02", Vec3f(3,0,4).length(), 5.0);
    a.checkEqual("03", Vec3f(0,3,4).length(), 5.0);
    a.checkEqual("04", Vec3f(0,0,0).length(), 0.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:norm", a)
{
    Vec3f v = Vec3f(5,0,0).norm();
    a.checkEqual("01", v(0), 1.0);
    a.checkEqual("02", v(1), 0.0);
    a.checkEqual("03", v(2), 0.0);

    v = Vec3f(3,0,4).norm();
    a.checkEqual("11", v(0), float(3.0/5.0)); // float() cast required to match precision
    a.checkEqual("12", v(1), 0.0);
    a.checkEqual("13", v(2), float(4.0/5.0));
}

AFL_TEST("gfx.threed.VecMath:Vec3f:prod", a)
{
    Vec3f v = Vec3f(1,0,0).prod(Vec3f(0,1,0));
    a.checkEqual("01", v(0), 0.0);
    a.checkEqual("02", v(1), 0.0);
    a.checkEqual("03", v(2), 1.0);

    v = Vec3f(3,4,5).prod(Vec3f(5,8,2));
    a.checkEqual("11", v(0), -32.0);
    a.checkEqual("12", v(1),  19.0);
    a.checkEqual("13", v(2),   4.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:mul", a)
{
    Vec3f v = Vec3f(10,15,20) * 3;
    a.checkEqual("01", v(0), 30.0);
    a.checkEqual("02", v(1), 45.0);
    a.checkEqual("03", v(2), 60.0);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:dot", a)
{
    // Perpendicular
    a.checkEqual("01", Vec3f(5,0,0).dot(Vec3f(0,6,0)), 0.0f);

    // Identical/Antiparallel
    a.checkEqual("11", Vec3f(0,0,3).dot(Vec3f(0,0,3)), 9.0f);
    a.checkEqual("12", Vec3f(0,0,3).dot(Vec3f(0,0,-3)), -9.0f);

    // Random
    a.checkEqual("21", Vec3f(1,2,3).dot(Vec3f(4,5,6)), 32.0f);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:per", a)
{
    Vec3f va(1,2,3);
    Vec3f vv = va.per();
    a.checkGreaterThan("01", vv.length(), 0.0f);
    a.checkEqual("02", va.dot(vv), 0.0f);

    va = Vec3f(4,4,4);
    vv = va.per();
    a.checkGreaterThan("11", vv.length(), 0.0f);
    a.checkEqual("12", va.dot(vv), 0.0f);
}

AFL_TEST("gfx.threed.VecMath:Vec3f:transform", a)
{
    // Identity transform
    Vec3f v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                           0,1,0,0,
                                           0,0,1,0,
                                           0,0,0,1));
    a.checkEqual("01", v(0), 7.0f);
    a.checkEqual("02", v(1), 8.0f);
    a.checkEqual("03", v(2), 9.0f);

    // Translation
    v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     10,20,30,1));
    a.checkEqual("11", v(0), 17.0f);
    a.checkEqual("12", v(1), 28.0f);
    a.checkEqual("13", v(2), 39.0f);

    // Scaling
    v = Vec3f(7,8,9).transform(Mat4f(2,0,0,0,
                                     0,3,0,0,
                                     0,0,4,0,
                                     0,0,0,1));
    a.checkEqual("21", v(0), 14.0f);
    a.checkEqual("22", v(1), 24.0f);
    a.checkEqual("23", v(2), 36.0f);

    // Perspective
    v = Vec3f(7,8,9).transform(Mat4f(1,0,0,0,
                                     0,1,0,0,
                                     0,0,1,0,
                                     0,0,0,2));
    a.checkEqual("31", v(0), 3.5f);
    a.checkEqual("32", v(1), 4.0f);
    a.checkEqual("33", v(2), 4.5f);
}

/*
 *  4D Vector
 */

AFL_TEST("gfx.threed.VecMath:Vec4f:make", a)
{
    Vec4f v = Vec4f(4,5,6,7);
    a.checkEqual("01", v(0), 4.0f);
    a.checkEqual("02", v(1), 5.0f);
    a.checkEqual("03", v(2), 6.0f);
    a.checkEqual("04", v(3), 7.0f);
}

/*
 *  Matrix
 */

AFL_TEST("gfx.threed.VecMath:Mat4f:make", a)
{
    Mat4f m = Mat4f();
    a.checkEqual("01", m(0), 0.0f);
    a.checkEqual("02", m(15), 0.0f);

    m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    a.checkEqual("11", m(0), 1.0f);
    a.checkEqual("12", m(15), 16.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:identity", a)
{
    Mat4f m = Mat4f::identity();
    a.checkEqual("01", m(0), 1.0f);
    a.checkEqual("02", m(1), 0.0f);
    a.checkEqual("03", m(15), 1.0f);

    Vec3f v = Vec3f(33,44,55).transform(m);
    a.checkEqual("11", v(0), 33.0f);
    a.checkEqual("12", v(1), 44.0f);
    a.checkEqual("13", v(2), 55.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:clone", a)
{
    // This test case is more meaningful in the JS version where we need to care about cloning.
    Mat4f m = Mat4f::identity();
    Mat4f n = m;
    n(0) = 7;
    a.checkEqual("01", m(0), 1.0f);
    a.checkEqual("02", n(0), 7.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:perspective:finite", a)
{
    Mat4f m = Mat4f::perspective(2, 1.5, 3.0, 100.0);

    a.checkNear("01", m(0), 0.42806, 0.00001);
    a.checkEqual("02", m(1), 0.0f);
    a.checkEqual("03", m(2), 0.0f);
    a.checkEqual("04", m(3), 0.0f);

    a.checkEqual("11", m(4), 0.0f);
    a.checkNear("12", m(5), 0.64209, 0.00001);
    a.checkEqual("13", m(6), 0.0f);
    a.checkEqual("14", m(7), 0.0f);

    a.checkEqual("21", m(8), 0.0f);
    a.checkEqual("22", m(9), 0.0f);
    a.checkNear("23", m(10), -1.061855, 0.000001);    // 103 / -97
    a.checkEqual("24", m(11), -1.0f);

    a.checkEqual("31", m(12), 0.0f);
    a.checkEqual("32", m(13), 0.0f);
    a.checkNear("33", m(14), -6.185567, 0.000001);    // 300 / -97 * 2
    a.checkEqual("34", m(15), 0.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:perspective:infinite", a)
{
    Mat4f m = Mat4f::perspective(2, 1.5, 3.0);

    a.checkNear("01", m(0), 0.42806, 0.00001);
    a.checkEqual("02", m(1), 0.0f);
    a.checkEqual("03", m(2), 0.0f);
    a.checkEqual("04", m(3), 0.0f);

    a.checkEqual("11", m(4), 0.0f);
    a.checkNear("12", m(5), 0.64209, 0.00001);
    a.checkEqual("13", m(6), 0.0f);
    a.checkEqual("14", m(7), 0.0f);

    a.checkEqual("21", m(8), 0.0f);
    a.checkEqual("22", m(9), 0.0f);
    a.checkEqual("23", m(10), -1.0f);
    a.checkEqual("24", m(11), -1.0f);

    a.checkEqual("31", m(12), 0.0f);
    a.checkEqual("32", m(13), 0.0f);
    a.checkEqual("33", m(14), -6.0f);
    a.checkEqual("34", m(15), 0.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:invert:singular", a)
{
    Mat4f m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    bool n = m.invert();
    a.checkEqual("01", n, false);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:invert:identity", a)
{
    Mat4f m = Mat4f::identity();
    bool n = m.invert();
    a.checkEqual("01", n, true);
    a.checkEqual("02", m(0),  1.0f);  a.checkEqual("02", m(1),  0.0f);  a.checkEqual("02", m(2),  0.0f);  a.checkEqual("02", m(3),  0.0f);
    a.checkEqual("03", m(4),  0.0f);  a.checkEqual("03", m(5),  1.0f);  a.checkEqual("03", m(6),  0.0f);  a.checkEqual("03", m(7),  0.0f);
    a.checkEqual("04", m(8),  0.0f);  a.checkEqual("04", m(9),  0.0f);  a.checkEqual("04", m(10), 1.0f);  a.checkEqual("04", m(11), 0.0f);
    a.checkEqual("05", m(12), 0.0f);  a.checkEqual("05", m(13), 0.0f);  a.checkEqual("05", m(14), 0.0f);  a.checkEqual("05", m(15), 1.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:invert:other", a)
{
    // Reference result obtained with Wolfram Alpha
    Mat4f m = Mat4f(1,2,3,4, 1,9,8,7, 5,60,7,80, 9,10,11,12);
    bool n = m.invert();
    a.checkEqual("01", n, true);
    a.checkNear("02", m(0), -133 / 504.0, 0.00001);  a.checkNear("02", m(1),  -56 / 504.0, 0.00001);  a.checkNear("02", m(2),   0 / 504.0, 0.00001);  a.checkNear("02", m(3),   77 / 504.0, 0.00001);
    a.checkNear("03", m(4), -246 / 504.0, 0.00001);  a.checkNear("03", m(5),   64 / 504.0, 0.00001);  a.checkNear("03", m(6),   4 / 504.0, 0.00001);  a.checkNear("03", m(7),   18 / 504.0, 0.00001);
    a.checkNear("04", m(8),  135 / 504.0, 0.00001);  a.checkNear("04", m(9),   40 / 504.0, 0.00001);  a.checkNear("04", m(10), -8 / 504.0, 0.00001);  a.checkNear("04", m(11), -15 / 504.0, 0.00001);
    a.checkNear("05", m(12), 181 / 504.0, 0.00001);  a.checkNear("05", m(13), -48 / 504.0, 0.00001);  a.checkNear("05", m(14),  4 / 504.0, 0.00001);  a.checkNear("05", m(15), -17 / 504.0, 0.00001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transpose", a)
{
    Mat4f m = Mat4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    Mat4f& n = m.transpose();
    a.checkEqual("01", &m, &n);
    a.checkEqual("02", m(0),  1.0f);  a.checkEqual("02", m(1),  5.0f);  a.checkEqual("02", m(2),   9.0f);  a.checkEqual("02", m(3),  13.0f);
    a.checkEqual("03", m(4),  2.0f);  a.checkEqual("03", m(5),  6.0f);  a.checkEqual("03", m(6),  10.0f);  a.checkEqual("03", m(7),  14.0f);
    a.checkEqual("04", m(8),  3.0f);  a.checkEqual("04", m(9),  7.0f);  a.checkEqual("04", m(10), 11.0f);  a.checkEqual("04", m(11), 15.0f);
    a.checkEqual("05", m(12), 4.0f);  a.checkEqual("05", m(13), 8.0f);  a.checkEqual("05", m(14), 12.0f);  a.checkEqual("05", m(15), 16.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:translate", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.translate(Vec3f(3,4,5));
    a.checkEqual("01", &m, &n);
    a.checkEqual("02", m(0),  1.0f);  a.checkEqual("02", m(1),  0.0f);  a.checkEqual("02", m(2),  0.0f);  a.checkEqual("02", m(3),  0.0f);
    a.checkEqual("03", m(4),  0.0f);  a.checkEqual("03", m(5),  1.0f);  a.checkEqual("03", m(6),  0.0f);  a.checkEqual("03", m(7),  0.0f);
    a.checkEqual("04", m(8),  0.0f);  a.checkEqual("04", m(9),  0.0f);  a.checkEqual("04", m(10), 1.0f);  a.checkEqual("04", m(11), 0.0f);
    a.checkEqual("05", m(12), 3.0f);  a.checkEqual("05", m(13), 4.0f);  a.checkEqual("05", m(14), 5.0f);  a.checkEqual("05", m(15), 1.0f);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkEqual("11", v(0), 13.0f);
    a.checkEqual("12", v(1), 24.0f);
    a.checkEqual("13", v(2), 35.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:scale:vector", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.scale(Vec3f(3,4,5));
    a.checkEqual("01", &m, &n);
    a.checkEqual("02", m(0),  3.0f);  a.checkEqual("02", m(1),  0.0f);  a.checkEqual("02", m(2),  0.0f);  a.checkEqual("02", m(3),  0.0f);
    a.checkEqual("03", m(4),  0.0f);  a.checkEqual("03", m(5),  4.0f);  a.checkEqual("03", m(6),  0.0f);  a.checkEqual("03", m(7),  0.0f);
    a.checkEqual("04", m(8),  0.0f);  a.checkEqual("04", m(9),  0.0f);  a.checkEqual("04", m(10), 5.0f);  a.checkEqual("04", m(11), 0.0f);
    a.checkEqual("05", m(12), 0.0f);  a.checkEqual("05", m(13), 0.0f);  a.checkEqual("05", m(14), 0.0f);  a.checkEqual("05", m(15), 1.0f);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkEqual("11", v(0), 30.0f);
    a.checkEqual("12", v(1), 80.0f);
    a.checkEqual("13", v(2), 150.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:scale:scalar", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.scale(6);
    a.checkEqual("01", &m, &n);
    a.checkEqual("02", m(0),  6.0f);  a.checkEqual("02", m(1),  0.0f);  a.checkEqual("02", m(2),  0.0f);  a.checkEqual("02", m(3),  0.0f);
    a.checkEqual("03", m(4),  0.0f);  a.checkEqual("03", m(5),  6.0f);  a.checkEqual("03", m(6),  0.0f);  a.checkEqual("03", m(7),  0.0f);
    a.checkEqual("04", m(8),  0.0f);  a.checkEqual("04", m(9),  0.0f);  a.checkEqual("04", m(10), 6.0f);  a.checkEqual("04", m(11), 0.0f);
    a.checkEqual("05", m(12), 0.0f);  a.checkEqual("05", m(13), 0.0f);  a.checkEqual("05", m(14), 0.0f);  a.checkEqual("05", m(15), 1.0f);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkEqual("11", v(0), 60.0f);
    a.checkEqual("12", v(1), 120.0f);
    a.checkEqual("13", v(2), 180.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:rotateX", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateX(util::PI/2);
    a.checkEqual("01", &m, &n);
    a.checkNear("02", m(0),  1, 0.000001f);  a.checkNear("02", m(1),  0, 0.000001f);  a.checkNear("02", m(2),  0, 0.000001f);  a.checkNear("02", m(3),  0, 0.000001f);
    a.checkNear("03", m(4),  0, 0.000001f);  a.checkNear("03", m(5),  0, 0.000001f);  a.checkNear("03", m(6),  1, 0.000001f);  a.checkNear("03", m(7),  0, 0.000001f);
    a.checkNear("04", m(8),  0, 0.000001f);  a.checkNear("04", m(9), -1, 0.000001f);  a.checkNear("04", m(10), 0, 0.000001f);  a.checkNear("04", m(11), 0, 0.000001f);
    a.checkNear("05", m(12), 0, 0.000001f);  a.checkNear("05", m(13), 0, 0.000001f);  a.checkNear("05", m(14), 0, 0.000001f);  a.checkNear("05", m(15), 1, 0.000001f);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkNear("11", v(0),  10, 0.000001f);
    a.checkNear("12", v(1), -30, 0.000001f);
    a.checkNear("13", v(2),  20, 0.000001f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:rotateY", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateY(util::PI/2);
    a.checkEqual("01", &m, &n);
    a.checkNear("02", m(0),  0, 0.000001);  a.checkNear("02", m(1),  0, 0.000001);  a.checkNear("02", m(2), -1, 0.000001);  a.checkNear("02", m(3),  0, 0.000001);
    a.checkNear("03", m(4),  0, 0.000001);  a.checkNear("03", m(5),  1, 0.000001);  a.checkNear("03", m(6),  0, 0.000001);  a.checkNear("03", m(7),  0, 0.000001);
    a.checkNear("04", m(8),  1, 0.000001);  a.checkNear("04", m(9),  0, 0.000001);  a.checkNear("04", m(10), 0, 0.000001);  a.checkNear("04", m(11), 0, 0.000001);
    a.checkNear("05", m(12), 0, 0.000001);  a.checkNear("05", m(13), 0, 0.000001);  a.checkNear("05", m(14), 0, 0.000001);  a.checkNear("05", m(15), 1, 0.000001);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkNear("11", v(0),  30, 0.000001);
    a.checkNear("12", v(1),  20, 0.000001);
    a.checkNear("13", v(2), -10, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:rotateZ", a)
{
    Mat4f m = Mat4f::identity();
    Mat4f& n = m.rotateZ(util::PI/2);
    a.checkEqual("01", &m, &n);
    a.checkNear("02", m(0),  0, 0.000001);  a.checkNear("02", m(1),  1, 0.000001);  a.checkNear("02", m(2),  0, 0.000001);  a.checkNear("02", m(3),  0, 0.000001);
    a.checkNear("03", m(4), -1, 0.000001);  a.checkNear("03", m(5),  0, 0.000001);  a.checkNear("03", m(6),  0, 0.000001);  a.checkNear("03", m(7),  0, 0.000001);
    a.checkNear("04", m(8),  0, 0.000001);  a.checkNear("04", m(9),  0, 0.000001);  a.checkNear("04", m(10), 1, 0.000001);  a.checkNear("04", m(11), 0, 0.000001);
    a.checkNear("05", m(12), 0, 0.000001);  a.checkNear("05", m(13), 0, 0.000001);  a.checkNear("05", m(14), 0, 0.000001);  a.checkNear("05", m(15), 1, 0.000001);

    Vec3f v = Vec3f(10,20,30).transform(m);
    a.checkNear("11", v(0), -20, 0.000001);
    a.checkNear("12", v(1),  10, 0.000001);
    a.checkNear("13", v(2),  30, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:mul-in-place", a)
{
    // Reference result obtained with Wolfram Alpha
    Mat4f m(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    Mat4f n(20,19,18,17, 16,15,14,13, 12,11,10,9, 8,7,6,5);
    Mat4f& prod = (m *= n);
    a.checkEqual("01", &m, &prod);
    a.checkEqual("02", m(0),  498.0f);  a.checkEqual("02", m(1),  572.0f);  a.checkEqual("02", m(2),  646.0f);  a.checkEqual("02", m(3),  720.0f);
    a.checkEqual("03", m(4),  386.0f);  a.checkEqual("03", m(5),  444.0f);  a.checkEqual("03", m(6),  502.0f);  a.checkEqual("03", m(7),  560.0f);
    a.checkEqual("04", m(8),  274.0f);  a.checkEqual("04", m(9),  316.0f);  a.checkEqual("04", m(10), 358.0f);  a.checkEqual("04", m(11), 400.0f);
    a.checkEqual("05", m(12), 162.0f);  a.checkEqual("05", m(13), 188.0f);  a.checkEqual("05", m(14), 214.0f);  a.checkEqual("05", m(15), 240.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:mul-infix", a)
{
    const Mat4f m(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16);
    const Mat4f n(20,19,18,17, 16,15,14,13, 12,11,10,9, 8,7,6,5);
    const Mat4f prod = (m * n);
    a.checkEqual("01", prod(0),  498.0f);  a.checkEqual("01", prod(1),  572.0f);  a.checkEqual("01", prod(2),  646.0f);  a.checkEqual("01", prod(3),  720.0f);
    a.checkEqual("02", prod(4),  386.0f);  a.checkEqual("02", prod(5),  444.0f);  a.checkEqual("02", prod(6),  502.0f);  a.checkEqual("02", prod(7),  560.0f);
    a.checkEqual("03", prod(8),  274.0f);  a.checkEqual("03", prod(9),  316.0f);  a.checkEqual("03", prod(10), 358.0f);  a.checkEqual("03", prod(11), 400.0f);
    a.checkEqual("04", prod(12), 162.0f);  a.checkEqual("04", prod(13), 188.0f);  a.checkEqual("04", prod(14), 214.0f);  a.checkEqual("04", prod(15), 240.0f);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:move-scale-rotate", a)
{
    Mat4f m = Mat4f::identity();
    m.rotateZ(util::PI/2);            // third operation
    m.scale(2);                       // second operation
    m.translate(Vec3f(50,40,30));     // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    a.checkNear("01", v(0), -92, 0.000001);
    a.checkNear("02", v(1), 110, 0.000001);
    a.checkNear("03", v(2),  74, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:move-scale", a)
{
    Mat4f m = Mat4f::identity();
    m.scale(2);                       // second operation
    m.translate(Vec3f(50,40,30));     // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    a.checkNear("01", v(0), 110, 0.000001);
    a.checkNear("02", v(1),  92, 0.000001);
    a.checkNear("03", v(2),  74, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:scale-move", a)
{
    Mat4f m = Mat4f::identity();
    m.translate(Vec3f(50,40,30));     // second operation
    m.scale(2);                       // first operation

    Vec3f v = Vec3f(5,6,7).transform(m);
    a.checkNear("01", v(0), 60, 0.000001);
    a.checkNear("02", v(1), 52, 0.000001);
    a.checkNear("03", v(2), 44, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:rotateX", a)
{
    Mat4f m = Mat4f::identity();
    m.rotateX(util::PI/4);

    Vec3f v = Vec3f(44,1,0).transform(m);
    a.checkNear("01", v(0), 44, 0.000001);
    a.checkNear("02", v(1), 0.707106, 0.000001);
    a.checkNear("03", v(2), 0.707106, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:rotateY", a)
{
    Mat4f m = Mat4f::identity();
    m.rotateY(util::PI/4);
    Vec3f v = Vec3f(1,44,0).transform(m);
    a.checkNear("01", v(0), 0.707106, 0.000001);
    a.checkNear("02", v(1), 44, 0.000001);
    a.checkNear("03", v(2), -0.707106, 0.000001);
}

AFL_TEST("gfx.threed.VecMath:Mat4f:transform:rotateZ", a)
{
    Mat4f m = Mat4f::identity();
    m.rotateZ(util::PI/4);
    Vec3f v = Vec3f(1,0,44).transform(m);
    a.checkNear("01", v(0), 0.707106, 0.000001);
    a.checkNear("02", v(1), 0.707106, 0.000001);
    a.checkNear("03", v(2), 44, 0.000001);
}
