/**
  *  \file test/gfx/threed/colortransformationtest.cpp
  *  \brief Test for gfx::threed::ColorTransformation
  */

#include "gfx/threed/colortransformation.hpp"
#include "afl/test/testrunner.hpp"

using gfx::threed::ColorTransformation;
using gfx::ColorQuad_t;

AFL_TEST("gfx.threed.ColorTransformation", a)
{
    const ColorQuad_t INPUT = COLORQUAD_FROM_RGBA(100, 200, 50, 130);

    // Identity transformation
    const ColorTransformation idTrans = ColorTransformation::identity();
    a.checkEqual("01", idTrans.transform(INPUT), INPUT);

    // Addition
    const ColorQuad_t ADDER = COLORQUAD_FROM_RGB(10, 20, 30);
    const ColorTransformation addTrans = ColorTransformation::identity().add(ADDER);
    a.checkEqual("11", addTrans.transform(INPUT), COLORQUAD_FROM_RGBA(110, 220, 80, 130));

    // Scaling
    const ColorTransformation scaleTrans = ColorTransformation::identity().scale(0.5);
    a.checkEqual("21", scaleTrans.transform(INPUT), COLORQUAD_FROM_RGBA(50, 100, 25, 130));

    // Add-then-scale
    const ColorQuad_t ADD_THEN_SCALE_OUT = COLORQUAD_FROM_RGBA(55, 110, 40, 130);
    a.checkEqual("31", (addTrans * scaleTrans).transform(INPUT), ADD_THEN_SCALE_OUT);
    a.checkEqual("32", scaleTrans.transform(addTrans.transform(INPUT)), ADD_THEN_SCALE_OUT);

    // Scale-then-add
    const ColorQuad_t SCALE_THEN_ADD_OUT = COLORQUAD_FROM_RGBA(60, 120, 55, 130);
    a.checkEqual("41", (scaleTrans * addTrans).transform(INPUT), SCALE_THEN_ADD_OUT);
    a.checkEqual("42", addTrans.transform(scaleTrans.transform(INPUT)), SCALE_THEN_ADD_OUT);
    a.checkEqual("43", ColorTransformation::identity().scale(0.5).add(ADDER).transform(INPUT), SCALE_THEN_ADD_OUT);

    ColorTransformation scaleThenAdd = scaleTrans;
    scaleThenAdd *= addTrans;
    a.checkEqual("51", scaleThenAdd.transform(INPUT), SCALE_THEN_ADD_OUT);

    // Grayscale
    a.checkEqual("61", ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255, 255, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153, 153, 153, 130));
    a.checkEqual("62", ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255,  85, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153,  51, 153, 130));
    a.checkEqual("63", ColorTransformation::toGrayscale(COLORQUAD_FROM_RGB(255,   0, 255)).transform(INPUT), COLORQUAD_FROM_RGBA(153,   0, 153, 130));

    // Addition
    a.checkEqual("71", (scaleTrans + scaleTrans).transform(INPUT), INPUT);

    ColorTransformation scaleAdded = scaleTrans;
    scaleAdded += scaleTrans;
    a.checkEqual("81", scaleAdded.transform(INPUT), INPUT);
}
