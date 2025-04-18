/**
  *  \file client/widgets/busyindicator.cpp
  */

#include "client/widgets/busyindicator.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

client::widgets::BusyIndicator::BusyIndicator(ui::Root& root, String_t text)
    : m_root(root),
      m_text(text),
      m_keys()
{ }
void
client::widgets::BusyIndicator::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Rectangle r(getExtent());
    drawSolidBar(ctx, r, ui::Color_Shield + 2);
    ui::drawFrameUp(ctx, r);

    ctx.setColor(ui::Color_White);
    r.grow(-2, -2);
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    ctx.useFont(*font);
    font->outText(ctx, r.getTopLeft(), m_text);
}

void
client::widgets::BusyIndicator::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::BusyIndicator::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
client::widgets::BusyIndicator::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    return gfx::Point(font->getTextWidth(m_text) + 4, font->getTextHeight(m_text) + 4);
}

bool
client::widgets::BusyIndicator::handleKey(util::Key_t key, int /*prefix*/)
{
    if (key == util::KeyMod_Ctrl + util::Key_Pause) {
        // Break
        m_keys.clear();
        sig_interrupt.raise();
    } else {
        // This loses the prefixes, but there shouldn't be any.
        m_keys.push_back(key);
    }
    return true;
}

bool
client::widgets::BusyIndicator::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return true;
}

void
client::widgets::BusyIndicator::replayEvents()
{
    while (!m_keys.empty()) {
        m_root.ungetKeyEvent(m_keys.back(), 0);
        m_keys.pop_back();
    }
}
