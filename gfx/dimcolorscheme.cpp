/**
  *  \file gfx/dimcolorscheme.cpp
  */

#include "gfx/dimcolorscheme.hpp"
#include "gfx/canvas.hpp"

/*
 *  FIXME: possible evolutions:
 *  - replace SkinColor by template?
 *  - make 0x55 configurable?
 *  - cache conversion results?
 */

gfx::DimColorScheme::DimColorScheme(gfx::ColorScheme<util::SkinColor::Color>& parent, Canvas& can)
    : m_parent(parent),
      m_canvas(can)
{ }

gfx::Color_t
gfx::DimColorScheme::getColor(util::SkinColor::Color index)
{
    Color_t colors[2] = {
        m_parent.getColor(util::SkinColor::Background),
        m_parent.getColor(index)
    };
    ColorQuad_t rgba[2];
    m_canvas.decodeColors(colors, rgba);

    ColorQuad_t rgbaResult[1] = { mixColor(rgba[0], rgba[1], 0x55) };
    Color_t colorResult[1];
    m_canvas.encodeColors(rgbaResult, colorResult);

    return colorResult[0];
}

void
gfx::DimColorScheme::drawBackground(Canvas& can, const Rectangle& area)
{
    m_parent.drawBackground(can, area);
}
