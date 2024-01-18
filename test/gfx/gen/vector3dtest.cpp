/**
  *  \file test/gfx/gen/vector3dtest.cpp
  *  \brief Test for gfx::gen::Vector3D
  */

#include "gfx/gen/vector3d.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test case. */
AFL_TEST("gfx.gen.Vector3D", a)
{
    typedef gfx::gen::Vector3D<int> IVec_t;

    IVec_t va(2, 3, 4);
    a.checkEqual("01. mag2", va.mag2(), 29);
    a.checkEqual("02", va.dot(IVec_t(7, 8, 9)), 74);

    IVec_t diff = va - IVec_t(5, 4, 3);
    a.checkEqual("11. x", diff.x, -3);
    a.checkEqual("12. y", diff.y, -1);
    a.checkEqual("13. z", diff.z, 1);

    IVec_t sum = va + IVec_t(32, 16, 8);
    a.checkEqual("21. x", sum.x, 34);
    a.checkEqual("22. y", sum.y, 19);
    a.checkEqual("23. z", sum.z, 12);

    IVec_t prod = va * 3;
    a.checkEqual("31. x", prod.x, 6);
    a.checkEqual("32. y", prod.y, 9);
    a.checkEqual("33. z", prod.z, 12);

    IVec_t z;
    a.checkEqual("41. mag2", z.mag2(), 0);
    a.checkEqual("42. x", z.x, 0);
    a.checkEqual("43. y", z.y, 0);
    a.checkEqual("44. z", z.z, 0);
}
