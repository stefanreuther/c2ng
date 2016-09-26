/**
  *  \file client/widgets/busyindicator.cpp
  */

#include "client/widgets/busyindicator.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

client::widgets::BusyIndicator::BusyIndicator(ui::Root& root, String_t text)
    : m_root(root),
      m_text(text)
{ }
void
client::widgets::BusyIndicator::draw(gfx::Canvas& can)
{
    gfx::Context ctx(can);
    gfx::Rectangle r(getExtent());
    ctx.useColorScheme(m_root.colorScheme());
    drawSolidBar(ctx, r, ui::Color_Shield + 2);
    ui::drawFrameUp(ctx, r);

    ctx.setColor(ui::Color_White);
    r.grow(-2, -2);
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    if (font.get() != 0) {
        ctx.useFont(*font);
        font->outText(ctx, r.getTopLeft(), m_text);
    }
}
void
client::widgets::BusyIndicator::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::BusyIndicator::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::BusyIndicator::getLayoutInfo() const
{
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    if (font.get() == 0) {
        return gfx::Point();
    } else {
        return gfx::Point(font->getTextWidth(m_text) + 4, font->getTextHeight(m_text) + 4);
    }
}

bool
client::widgets::BusyIndicator::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return true;
}

bool
client::widgets::BusyIndicator::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return true;
}
