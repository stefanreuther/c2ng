/**
  *  \file ui/reshack/linetool.cpp
  *  \brief Class ui::reshack::LineTool
  */

#include "ui/reshack/linetool.hpp"
#include "gfx/complex.hpp"

ui::reshack::LineTool::LineTool(afl::string::Translator& tx)
    : Tool(true, tx("Line")), m_prev()
{ }

void
ui::reshack::LineTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // ex RHLineTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    m_prev = pt;
    drawPixel(c, pt);
}

void
ui::reshack::LineTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    drawLine(c, m_prev, pt);
}

void
ui::reshack::LineTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    LineTool::drag(c, pt);
}

bool
ui::reshack::LineTool::isUsable()
{
    return true;
}
