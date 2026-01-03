/**
  *  \file ui/widgets/busyindicator.cpp
  */

#include "ui/widgets/busyindicator.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

ui::widgets::BusyIndicator::BusyIndicator(Root& root, String_t text)
    : m_root(root),
      m_text(text),
      m_keys(),
      m_quit(false)
{ }
void
ui::widgets::BusyIndicator::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    gfx::Rectangle r(getExtent());
    drawSolidBar(ctx, r, Color_Shield + 2);
    drawFrameUp(ctx, r);

    ctx.setColor(Color_White);
    r.grow(-2, -2);
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    ctx.useFont(*font);
    font->outText(ctx, r.getTopLeft(), m_text);
}

void
ui::widgets::BusyIndicator::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::BusyIndicator::handlePositionChange()
{
    requestRedraw();
}

ui::layout::Info
ui::widgets::BusyIndicator::getLayoutInfo() const
{
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    return gfx::Point(font->getTextWidth(m_text) + 4, font->getTextHeight(m_text) + 4);
}

bool
ui::widgets::BusyIndicator::handleKey(util::Key_t key, int /*prefix*/)
{
    if (key == util::KeyMod_Ctrl + util::Key_Pause) {
        // Break
        m_keys.clear();
        sig_interrupt.raise();
    } else if (key == util::Key_Quit) {
        // Quit
        m_quit = true;
        m_keys.clear();
        sig_quit.raise();
    } else {
        // This loses the prefixes, but there shouldn't be any.
        if (!m_quit) {
            m_keys.push_back(key);
        }
    }
    return true;
}

bool
ui::widgets::BusyIndicator::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return true;
}

void
ui::widgets::BusyIndicator::replayEvents()
{
    while (!m_keys.empty()) {
        m_root.ungetKeyEvent(m_keys.back(), 0);
        m_keys.pop_back();
    }
    if (m_quit) {
        m_root.ungetKeyEvent(util::Key_Quit, 0);
        m_quit = false;
    }
}
