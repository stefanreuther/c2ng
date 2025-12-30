/**
  *  \file ui/reshack/pipettetool.cpp
  *  \brief Class ui::reshack::PipetteTool
  */

#include "ui/reshack/pipettetool.hpp"
#include "gfx/canvas.hpp"
#include "ui/reshack/painter.hpp"

ui::reshack::PipetteTool::PipetteTool(afl::string::Translator& tx, Painter& p)
    : Tool(false, tx("Pipette")),
      m_painter(p)
{ }

void
ui::reshack::PipetteTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // RHPipetteTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    gfx::Color_t color[1];
    c.canvas().getPixels(pt, color);
    m_painter.setColor(false, static_cast<uint8_t>(color[0]));
}

void
ui::reshack::PipetteTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    PipetteTool::click(c, pt, 0);
}

void
ui::reshack::PipetteTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    PipetteTool::click(c, pt, 0);
}

bool
ui::reshack::PipetteTool::isUsable()
{
    return true;
}
