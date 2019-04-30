/**
  *  \file client/map/prefixoverlay.cpp
  *
  *  FIXME: reconsider whether we actually need this guy.
  *  What it does is keep mouse movement working while the user types a prefix.
  *  If we don't use that, we could also use a normal ui::PrefixArgument.
  */

#include "client/map/prefixoverlay.hpp"
#include "client/map/callback.hpp"
#include "client/map/renderer.hpp"
#include "client/map/screen.hpp"
#include "gfx/complex.hpp"
#include "ui/colorscheme.hpp"
#include "ui/draw.hpp"

const afl::sys::Timeout_t BLINK_INTERVAL = 400;

client::map::PrefixOverlay::PrefixOverlay(Screen& screen, int initialValue)
    : m_screen(screen),
      m_value(initialValue),
      m_timer(screen.root().engine().createTimer()),
      m_blink(false)
{
    // ex WPrefixChartMode::WPrefixChartMode
    m_timer->sig_fire.add(this, &PrefixOverlay::onTimer);
    m_timer->setInterval(BLINK_INTERVAL);
}

client::map::PrefixOverlay::~PrefixOverlay()
{ }

void
client::map::PrefixOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::PrefixOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WPrefixChartMode::drawOverlays

    // Figure out location
    gfx::Point pt = ren.getExtent().getCenter();
    String_t text = m_value.getText(m_screen.translator());

    // Font metrics
    afl::base::Ref<gfx::Font> font = m_screen.root().provider().getFont(gfx::FontRequest().setStyle(ui::FixedFont));
    int wi = font->getTextWidth(text) + 14;
    int he = font->getTextHeight(text) + 2;

    gfx::Rectangle r(pt.getX() - wi/2, pt.getY() - he/2 + 20, wi, he);

    // Draw.
    gfx::Context<uint8_t> ctx(can, m_screen.root().colorScheme());
    drawSolidBar(ctx, r, ui::Color_Fire + 29);

    ctx.setColor(ui::Color_DarkYellow);
    drawHLine(ctx, r.getLeftX(), r.getBottomY()-1, r.getRightX()-1);
    drawVLine(ctx, r.getRightX()-1, r.getTopY(), r.getBottomY()-2);

    ctx.setColor(ui::Color_Fire + 30);
    drawHLine(ctx, r.getLeftX()+1, r.getTopY(), r.getRightX()-1);
    drawVLine(ctx, r.getLeftX(), r.getTopY(), r.getBottomY()-2);

    ctx.setColor(ui::Color_Black);
    ctx.useFont(*font);
    outText(ctx, gfx::Point(r.getLeftX() + 3, r.getTopY() + 1), text);

    if (!m_blink) {
        drawSolidBar(ctx, gfx::Rectangle(r.getRightX() - 10, r.getBottomY() - 5, 7, 2), ui::Color_Black);
    }
}

bool
client::map::PrefixOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return true;
}

bool
client::map::PrefixOverlay::handleKey(util::Key_t key, int /*prefix*/, const Renderer& /*ren*/)
{
    // ex WPrefixChartMode::handleEvent
    switch (m_value.handleKey(key)) {
     case util::PrefixArgument::Accepted:
        if (client::map::Callback* pCB = getCallback()) {
            pCB->requestRedraw();
        }
        return true;

     case util::PrefixArgument::Canceled:
        m_screen.setNewOverlay(Screen::PrefixLayer, 0);
        return true;

     case util::PrefixArgument::NotHandled:
        if (key == util::Key_Quit) {
            // Quit. Treat as cancel and don't bother with the prefix.
            finish(m_screen, key, 0);
            return true;
        } else if (util::classifyKey(key & util::Key_Mask) == util::NormalKey) {
            // Accepted key: kill ourselves and dispatch it
            finish(m_screen, key, m_value.getValue());
            return true;
        } else {
            // It's a shift code. Maybe someone wants to track it. Why not.
            return false;
        }
    }
    return false;
}

bool
client::map::PrefixOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    // Do not handle mouse. PrefixOverlay is intended for starchart use only.
    // Other maps use a regular ui::PrefixArgument.
    return false;
}

void
client::map::PrefixOverlay::onTimer()
{
    m_blink = !m_blink;
    if (client::map::Callback* pCB = getCallback()) {
        pCB->requestRedraw();
    }
    m_timer->setInterval(BLINK_INTERVAL);
}

void
client::map::PrefixOverlay::finish(Screen& screen, util::Key_t key, int prefix)
{
    // This is a static function so nobody accidentally dereferences *this after it died.
    screen.setNewOverlay(Screen::PrefixLayer, 0);
    screen.handleKey(key, prefix);
}
