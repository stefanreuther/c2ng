/**
  *  \file ui/skincolorscheme.cpp
  */

#include "ui/skincolorscheme.hpp"
#include "ui/draw.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"

ui::SkinColorScheme::SkinColorScheme(const ColorSet& colors, ColorScheme& uiColorScheme)
    : m_colors(colors),
      m_uiColorScheme(uiColorScheme)
{ }

gfx::Color_t
ui::SkinColorScheme::getColor(uint32_t index)
{
    if (index < SkinColor::NUM_COLORS) {
        return m_uiColorScheme.getColor(m_colors[SkinColor::Color(index)]);
    } else {
        return m_uiColorScheme.getColor(0);
    }
}

void
ui::SkinColorScheme::drawBackground(gfx::Context& ctx, const gfx::Rectangle& area)
{
    ctx.canvas().drawBar(area, getColor(SkinColor::Background),
                         gfx::TRANSPARENT_COLOR,
                         gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
                         
}
