/**
  *  \file client/map/drawlineoverlay.cpp
  */

#include "client/map/drawlineoverlay.hpp"
#include "gfx/context.hpp"
#include "client/map/location.hpp"
#include "client/map/renderer.hpp"
#include "client/map/screen.hpp"

client::map::DrawLineOverlay::DrawLineOverlay(ui::Root& root,
                afl::string::Translator& tx,
                client::map::Location& loc,
                client::map::Screen& screen,
                const game::map::Drawing& drawing)
    : MarkerOverlayBase(root, tx, screen, drawing),
      m_location(loc),
      conn_positionChange(loc.sig_positionChange.add(this, &DrawLineOverlay::onPositionChange))
{ }

void
client::map::DrawLineOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::DrawLineOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDrawMarkerChartMode::drawOverlays
    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*root().provider().getFont(gfx::FontRequest()));
    ctx.setColor(ui::Color_White);

    outText(ctx, ren.getExtent().getTopLeft(), translator()("Mark 2nd point, end with [ESC] or [+]."));
}

bool
client::map::DrawLineOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::DrawLineOverlay::handleKey(util::Key_t key, int prefix, const Renderer& ren)
{
    switch (key) {
     case '+':
        if (drawing().getType() == game::map::Drawing::LineDrawing) {
            // Line: start new line
            screen().drawingProxy().continueLine();
        } else {
            // Rectangle: exit move mode (or marker deleted)
            screen().removeOverlay(this);
        }
        return true;

     case '-':
        // No effect, because we handle '+' specially. Avoids that users
        // zoom out and cannot zoom in.
        return true;

     case util::Key_Backspace:
        if (drawing().getPos2() != drawing().getPos()) {
            // Go to start point
            m_location.setPosition(drawing().getPos());
        } else {
            // Trying to backspace from zero-size drawing, so exit mode
            screen().removeOverlay(this);
        }
        return true;

     case 'x': {
        // Go to start point, swapping beginning and end of drawing
        const game::map::Point a = drawing().getPos(), b = drawing().getPos2();
        screen().drawingProxy().setPos(b);
        screen().drawingProxy().setPos2(a);
        m_location.setPosition(a);
        return true;
     }

     case 'p':
        screen().removeOverlay(this);
        return true;

     default:
        return defaultHandleKey(key, prefix, ren);
    }
}

bool
client::map::DrawLineOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::DrawLineOverlay::onPositionChange(game::map::Point pt)
{
    // ex WDrawMarkerChartMode::onMove
    // FIXME: handle map seam crossong
    // GPoint adj = loc.getWrapAdjust();
    // if (adj.x || adj.y) {
    //     (*drawing)->setPos((*drawing)->getPos() + adj);
    //     loc.markChanged();
    // }

    screen().drawingProxy().setPos2(pt);
}
