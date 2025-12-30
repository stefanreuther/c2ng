/**
  *  \file ui/reshack/brushtool.cpp
  *  \brief Class ui::reshack::BrushTool
  */

#include "ui/reshack/brushtool.hpp"
#include "gfx/complex.hpp"

ui::reshack::BrushTool::BrushTool(afl::string::Translator& tx)
    : Tool(false, tx("Brush"))
{ }

void
ui::reshack::BrushTool::click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t /*bg*/)
{
    // ex RHBrushTool::click(GfxContext& c, GfxPoint pt, uint32_t /*bg*/)
    drawFilledCircle(c, pt, 5);
}

void
ui::reshack::BrushTool::drag(gfx::BaseContext& c, gfx::Point pt)
{
    drawFilledCircle(c, pt, 5);
}

void
ui::reshack::BrushTool::release(gfx::BaseContext& c, gfx::Point pt)
{
    drawFilledCircle(c, pt, 5);
}

bool
ui::reshack::BrushTool::isUsable()
{
    return true;
}
