/**
  *  \file client/map/circleoverlay.cpp
  *  \brief Class client::map::CircleOverlay
  */

#include "client/map/circleoverlay.hpp"
#include "client/map/renderer.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/updater.hpp"

/*
 *  Porting Note: a WObjectSelectionChartWidget is a client::map::Widget containing a CircleOverlay
 *  with Color_Gray, tracking position and radius of a game::map::CircularObject, and auto-adjusting zoom.
 */


client::map::CircleOverlay::CircleOverlay(ui::ColorScheme& colorScheme)
    : m_colorScheme(colorScheme),
      m_center(),
      m_radius(0),
      m_color()
{ }

client::map::CircleOverlay::~CircleOverlay()
{ }

void
client::map::CircleOverlay::setPosition(game::map::Point center, int radius)
{
    if (util::Updater().set(m_center, center).set(m_radius, radius)) {
        requestRedraw();
    }
}

void
client::map::CircleOverlay::setColor(uint8_t color)
{
    if (util::Updater().set(m_color, color)) {
        requestRedraw();
    }
}

void
client::map::CircleOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::CircleOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WObjectSelectionChartWidget::drawData (sort-of)
    if (m_radius > 0) {
        gfx::Context<uint8_t> ctx(can, m_colorScheme);
        ctx.setColor(m_color);
        drawCircle(ctx, ren.scale(m_center), ren.scale(m_radius));
    }
}

bool
client::map::CircleOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::CircleOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::CircleOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}
