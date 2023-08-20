/**
  *  \file u/t_gfx_threed_colortransformation.cpp
  *  \brief Test for gfx::threed::ColorTransformation
  */

#include "gfx/threed/colortransformation.hpp"

#include "t_gfx_threed.hpp"

using gfx::threed::ColorTransformation;
using gfx::ColorQuad_t;

void
TestGfxThreedColorTransformation::testIt()
{
    const ColorQuad_t INPUT = COLORQUAD_FROM_RGBA(100, 200, 50, 130);

    // Identity transformation
    const ColorTransformation idTrans = ColorTransformation::identity();
    TS_ASSERT_EQUALS(idTrans.transform(INPUT), INPUT);

    // Addition
    const ColorQuad_t ADDER = COLORQUAD_FROM_RGB(10, 20, 30);
    const ColorTransformation addTrans = ColorTransformation::identity().add(ADDER);
    TS_ASSERT_EQUALS(addTrans.transform(INPUT), COLORQUAD_FROM_RGBA(110, 220, 80, 130));

    // Scaling
    const ColorTransformation scaleTrans = ColorTransformation::identity().scale(0.5);
    TS_ASSERT_EQUALS(scaleTrans.transform(INPUT), COLORQUAD_FROM_RGBA(50, 100, 25, 130));

    // Add-then-scale
    const ColorQuad_t ADD_THEN_SCALE_OUT = COLORQUAD_FROM_RGBA(55, 110, 40, 130);
    TS_ASSERT_EQUALS((addTrans * scaleTrans).transform(INPUT), ADD_THEN_SCALE_OUT);
    TS_ASSERT_EQUALS(scaleTrans.transform(addTrans.transform(INPUT)), ADD_THEN_SCALE_OUT);

    // Scale-then-add
    const ColorQuad_t SCALE_THEN_ADD_OUT = COLORQUAD_FROM_RGBA(60, 120, 55, 130);
    TS_ASSERT_EQUALS((scaleTrans * addTrans).transform(INPUT), SCALE_THEN_ADD_OUT);
    TS_ASSERT_EQUALS(addTrans.transform(scaleTrans.transform(INPUT)), SCALE_THEN_ADD_OUT);
    TS_ASSERT_EQUALS(ColorTransformation::identity().scale(0.5).add(ADDER).transform(INPUT), SCALE_THEN_ADD_OUT);

    ColorTransformation scaleThenAdd = scaleTrans;
    scaleThenAdd *= addTrans;
    TS_ASSERT_EQUALS(scaleThenAdd.transform(INPUT), SCALE_THEN_ADD_OUT);

    // Grayscale
    TS_ASSERT_EQUALS(ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255, 255, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153, 153, 153, 130));
    TS_ASSERT_EQUALS(ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255,  85, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153,  51, 153, 130));
    TS_ASSERT_EQUALS(ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255,   0, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153,   0, 153, 130));

    // Addition
    TS_ASSERT_EQUALS((scaleTrans + scaleTrans).transform(INPUT), INPUT);

    ColorTransformation scaleAdded = scaleTrans;
    scaleAdded += scaleTrans;
    TS_ASSERT_EQUALS(scaleAdded.transform(INPUT), INPUT);
}
