/**
  *  \file ui/reshack/circletool.cpp
  *  \brief Class ui::reshack::CircleTool
  */

#include "ui/reshack/circletool.hpp"
#include "gfx/complex.hpp"
#include "util/math.hpp"


ui::reshack::CircleTool::CircleTool(afl::string::Translator& tx)
    : Tool(true, tx("Circle")),
      m_prev()
{ }

void
ui::reshack::CircleTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // RHCircleTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    m_prev = pt;
    drawPixel(c, pt);
}

void
ui::reshack::CircleTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    // RHCircleTool::drag(GfxContext& c, GfxPoint pt)
    int radius = int(util::getDistanceFromDX(pt.getX() - m_prev.getX(), pt.getY() - m_prev.getY()) + 0.5);
    drawCircle(c, m_prev, radius);
}

void
ui::reshack::CircleTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    CircleTool::drag(c, pt);
}

bool
ui::reshack::CircleTool::isUsable()
{
    return true;
}
