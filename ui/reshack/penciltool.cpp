/**
  *  \file ui/reshack/penciltool.cpp
  *  \brief Class ui::reshack::PencilTool
  */

#include "ui/reshack/penciltool.hpp"
#include "gfx/complex.hpp"

ui::reshack::PencilTool::PencilTool(afl::string::Translator& tx)
    : Tool(false, tx("Pencil")),
      m_prev()
{ }

void
ui::reshack::PencilTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // ex RHPencilTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    m_prev = pt;
    drawPixel(c, pt);
}

void
ui::reshack::PencilTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    // RHPencilTool::drag(GfxContext& c, GfxPoint pt)
    drawLine(c, m_prev, pt);
    m_prev = pt;
}

void
ui::reshack::PencilTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    // RHPencilTool::release(GfxContext& c, GfxPoint pt)
    PencilTool::drag(c, pt);
}

bool
ui::reshack::PencilTool::isUsable()
{
    return true;
}
