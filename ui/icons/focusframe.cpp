/**
  *  \file ui/icons/focusframe.cpp
  */

#include "ui/icons/focusframe.hpp"
#include "gfx/complex.hpp"
#include "util/updater.hpp"

ui::icons::FocusFrame::FocusFrame(Icon& content)
    : Icon(),
      m_content(content),
      m_pad(2)
{ }

ui::icons::FocusFrame::~FocusFrame()
{ }

bool
ui::icons::FocusFrame::setPad(int pad)
{
    return util::Updater().set(m_pad, pad);
}

gfx::Point
ui::icons::FocusFrame::getSize() const
{
    return m_content.getSize() + gfx::Point(2*m_pad, 2*m_pad);
}

void
ui::icons::FocusFrame::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    if (flags.contains(FocusedButton)) {
        ctx.setColor(SkinColor::Static);
        drawRectangle(ctx, area);
    }

    area.grow(-m_pad, -m_pad);
    m_content.draw(ctx, area, flags);
}
