/**
  *  \file u/t_gfx_nullcanvas.cpp
  *  \brief Test for gfx::NullCanvas
  */

#include "gfx/nullcanvas.hpp"

#include "t_gfx.hpp"

/** Simple test. */
void
TestGfxNullCanvas::testIt()
{
    // NullCanvas does not do anything.
    // But the object must be creatible.
    gfx::NullCanvas testee;
}

