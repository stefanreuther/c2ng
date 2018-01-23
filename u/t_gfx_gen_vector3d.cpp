/**
  *  \file u/t_gfx_gen_vector3d.cpp
  *  \brief Test for gfx::gen::Vector3D
  */

#include "gfx/gen/vector3d.hpp"

#include "t_gfx_gen.hpp"

/** Simple test case. */
void
TestGfxGenVector3D::testIt()
{
    typedef gfx::gen::Vector3D<int> IVec_t;

    IVec_t a(2, 3, 4);
    TS_ASSERT_EQUALS(a.mag2(), 29);
    TS_ASSERT_EQUALS(a.dot(IVec_t(7, 8, 9)), 74);

    IVec_t diff = a - IVec_t(5, 4, 3);
    TS_ASSERT_EQUALS(diff.x, -3);
    TS_ASSERT_EQUALS(diff.y, -1);
    TS_ASSERT_EQUALS(diff.z, 1);

    IVec_t sum = a + IVec_t(32, 16, 8);
    TS_ASSERT_EQUALS(sum.x, 34);
    TS_ASSERT_EQUALS(sum.y, 19);
    TS_ASSERT_EQUALS(sum.z, 12);

    IVec_t prod = a * 3;
    TS_ASSERT_EQUALS(prod.x, 6);
    TS_ASSERT_EQUALS(prod.y, 9);
    TS_ASSERT_EQUALS(prod.z, 12);

    IVec_t z;
    TS_ASSERT_EQUALS(z.mag2(), 0);
    TS_ASSERT_EQUALS(z.x, 0);
    TS_ASSERT_EQUALS(z.y, 0);
    TS_ASSERT_EQUALS(z.z, 0);
}

