/**
  *  \file ui/reshack/rectangletool.cpp
  *  \brief Class ui::reshack::RectangleTool
  */

#include "ui/reshack/rectangletool.hpp"

#include <algorithm>
#include "gfx/complex.hpp"

ui::reshack::RectangleTool::RectangleTool(afl::string::Translator& tx, bool solid)
    : Tool(true, solid ? tx("Solid Bar") : tx("Rectangle")),
      m_solid(solid),
      m_prev()
{ }

void
ui::reshack::RectangleTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // RHRectangleTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    // RHBarTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    m_prev = pt;
    drawPixel(c, pt);
}

void
ui::reshack::RectangleTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    if (m_solid) {
        c.setFillPattern(gfx::FillPattern::SOLID);
        drawBar(c, makeRectangle(pt, m_prev));
    } else {
        drawRectangle(c, makeRectangle(pt, m_prev));
    }
}

void
ui::reshack::RectangleTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    RectangleTool::drag(c, pt);
}

bool
ui::reshack::RectangleTool::isUsable()
{
    return true;
}

gfx::Rectangle
ui::reshack::RectangleTool::makeRectangle(gfx::Point a, gfx::Point b)
{
    int x = std::min(a.getX(), b.getX());
    int y = std::min(a.getY(), b.getY());
    int w = std::max(a.getX(), b.getX()) - x + 1;
    int h = std::max(a.getY(), b.getY()) - y + 1;
    return gfx::Rectangle(x, y, w, h);
}
