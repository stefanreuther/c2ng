/**
  *  \file ui/skincolorscheme.cpp
  */

#include "ui/skincolorscheme.hpp"
#include "ui/draw.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"

ui::SkinColorScheme::SkinColorScheme(const ColorSet& colors, ui::ColorScheme& uiColorScheme)
    : m_colors(colors),
      m_uiColorScheme(uiColorScheme)
{ }

gfx::Color_t
ui::SkinColorScheme::getColor(util::SkinColor::Color index)
{
    if (size_t(index) < SkinColor::NUM_COLORS) {
        return m_uiColorScheme.getColor(m_colors[index]);
    } else {
        return m_uiColorScheme.getColor(0);
    }
}

void
ui::SkinColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    can.drawBar(area, getColor(SkinColor::Background), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
}
