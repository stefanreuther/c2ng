/**
  *  \file client/map/messageoverlay.cpp
  *  \brief Class client::map::MessageOverlay
  */

#include "client/map/messageoverlay.hpp"
#include "client/map/screen.hpp"
#include "gfx/context.hpp"

const int MAX_STATE = 8;

client::map::MessageOverlay::MessageOverlay(Screen& parent, String_t message)
    : m_parent(parent),
      m_message(message),
      m_timer(parent.root().engine().createTimer()),
      m_state(0)
{
    m_timer->sig_fire.add(this, &MessageOverlay::onTimer);
    startTimer();
}

client::map::MessageOverlay::~MessageOverlay()
{ }

// Overlay:
void
client::map::MessageOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::MessageOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WMessageChartMode::drawOverlays
    gfx::Context<uint8_t> ctx(can, m_parent.root().colorScheme());
    ctx.useFont(*m_parent.root().provider().getFont("+"));
    ctx.setColor(ui::Color_Dark);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);

    // "Frame"
    gfx::Point pt = ren.getExtent().getCenter();
    for (int i = -1; i <= +1; ++i) {
        for (int j = -1; j <= +1; ++j) {
            if (i != 0 || j != 0) {
                outText(ctx, pt + gfx::Point(i, j), m_message);
            }
        }
    }

    // "Text"
    ctx.setColor(uint8_t(ui::Color_Grayscale+15 - std::min(m_state, MAX_STATE)));
    outText(ctx, pt, m_message);
}

bool
client::map::MessageOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MessageOverlay::handleKey(util::Key_t key, int prefix, const Renderer& /*ren*/)
{
    // ex WMessageChartMode::handleEvent (part)
    if (util::classifyKey(key & util::Key_Mask) != util::ModifierKey) {
        // Re-post the key, then remove this overlay.
        // Cannot just discard this overlay and return false, because we're in an iteration of overlay here.
        m_parent.root().ungetKeyEvent(key, prefix);
        m_parent.removeOverlay(this);
        return true;
    } else {
        // Swallow
        return true;
    }
}

bool
client::map::MessageOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    // ex WMessageChartMode::handleEvent (part)
    // @change The "close if mouse clicked" logic is in client::map::Screen::handleMouseRelative();
    // Screen doesn't dispatch mouse events through this path.
    return false;
}

void
client::map::MessageOverlay::startTimer()
{
    if (m_state == 0) {
        m_timer->setInterval(500);
    } else {
        m_timer->setInterval(75);
    }
}

void
client::map::MessageOverlay::onTimer()
{
    ++m_state;
    if (m_state >= MAX_STATE) {
        m_parent.removeOverlay(this);
    } else {
        startTimer();
        if (Callback* cb = getCallback()) {
            cb->requestRedraw();
        }
    }
}
