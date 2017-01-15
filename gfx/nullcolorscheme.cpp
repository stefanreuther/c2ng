/**
  *  \file gfx/nullcolorscheme.cpp
  */

#include "gfx/nullcolorscheme.hpp"
#include "gfx/context.hpp"
#include "gfx/canvas.hpp"
#include "gfx/fillpattern.hpp"

// gfx::NullColorScheme gfx::NullColorScheme::instance;

// gfx::NullColorScheme::~NullColorScheme()
// { }

// gfx::Color_t
// gfx::NullColorScheme::getColor(uint32_t index)
// {
//     return Color_t(index);
// }

// void
// gfx::NullColorScheme::drawBackground(Context& ctx, const Rectangle& area)
// {
//     if (!ctx.isTransparentBackground()) {
//         ctx.canvas().drawBar(area, 0, TRANSPARENT_COLOR, FillPattern::SOLID, OPAQUE_ALPHA);
//     }
// }

void
gfx::drawNullBackground(Canvas& can, const Rectangle& area)
{
    can.drawBar(area, 0, TRANSPARENT_COLOR, FillPattern::SOLID, OPAQUE_ALPHA);
}
