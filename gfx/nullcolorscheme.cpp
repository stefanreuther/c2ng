/**
  *  \file gfx/nullcolorscheme.cpp
  *  \brief Class gfx::NullColorScheme
  */

#include "gfx/nullcolorscheme.hpp"
#include "gfx/context.hpp"
#include "gfx/canvas.hpp"
#include "gfx/fillpattern.hpp"

void
gfx::drawNullBackground(Canvas& can, const Rectangle& area)
{
    can.drawBar(area, 0, TRANSPARENT_COLOR, FillPattern::SOLID, OPAQUE_ALPHA);
}
