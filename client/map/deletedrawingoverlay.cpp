/**
  *  \file client/map/deletedrawingoverlay.cpp
  */

#include "client/map/deletedrawingoverlay.hpp"
#include "client/map/screen.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"

client::map::DeleteDrawingOverlay::DeleteDrawingOverlay(ui::Root& root,
                                                        afl::string::Translator& tx,
                                                        client::map::Screen& screen,
                                                        const game::map::Drawing& drawing)
    : MarkerOverlayBase(root, tx, screen, drawing),
      m_phase(false),
      m_timer(root.engine().createTimer())
{
    m_timer->sig_fire.add(this, &DeleteDrawingOverlay::onTimer);
    m_timer->setInterval(ui::CURSOR_BLINK_INTERVAL);
}

void
client::map::DeleteDrawingOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::DeleteDrawingOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDeleteDrawingChartMode::drawOverlays
    // WMarkerChartMode::drawOverlays(can, loc);
    const afl::base::Ref<gfx::Font> font = root().provider().getFont(gfx::FontRequest());
    const String_t text = translator()("Delete this drawing (y/n)?");

    const int width = font->getTextWidth(text) + 20;
    const int height = font->getTextHeight(text);

    const gfx::Point center = ren.getExtent().getCenter();
    const gfx::Rectangle area(center.getX() - width/2, center.getY() + height*5/4, width, height);

    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*font);
    ctx.setColor(ui::Color_White);

    drawSolidBar(ctx, area, ui::Color_Red);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    outTextF(ctx, area, text);

    if (drawing().getType() == game::map::Drawing::LineDrawing) {
        const String_t text2 = translator()("Press [A] to delete adjacent lines, too.");
        const int width2 = font->getTextWidth(text2) + 20;

        const gfx::Rectangle area2(center.getX() - width2/2, area.getBottomY(), width2, height);

        drawSolidBar(ctx, area2, ui::Color_Red);
        outTextF(ctx, area2, text2);
    }
}

bool
client::map::DeleteDrawingOverlay::drawCursor(gfx::Canvas& can, const Renderer& ren)
{
    // ex WDeleteDrawingChartMode::drawCursor
    // Note that the colors are user colors for now: 4=red, 10=white
    ren.drawDrawing(can, root().colorScheme(), root().provider(), drawing(), m_phase ? 4 : 10);
    return false;
}

bool
client::map::DeleteDrawingOverlay::handleKey(util::Key_t key, int /*prefix*/, const Renderer& /*ren*/)
{
    // ex WDeleteDrawingChartMode::handleEvent
    /* In PCC 1.x, Delete Mode eats all keyboard input except for
       (shifted) arrows, digits, and dot. Delete mode itself offers
       y (RET, SPC) and n (ESC), DEL to pick another object, and a
       to delete adjacent objects. */
    switch (key) {
     case 'y':
     case util::Key_Return:
     case ' ':
        screen().drawingProxy().erase(false);
        screen().removeOverlay(this);
        return true;

     case 'n':
     case util::Key_Escape:
     case util::Key_Quit:
        // Do not delete
        // FIXME: Key_Quit should probably re-post. However, this is a very minor use-case,
        // because by default we grab the mouse and therefore vk_Quit cannot be generated.
        screen().removeOverlay(this);
        return true;

     case util::Key_Delete:
        // Try to find a new drawing. If none found, keeps the previous one.
        screen().selectNearestVisibleDrawing();
        return true;

     case 'a':
        // Delete adjacent
        if (drawing().getType() == game::map::Drawing::LineDrawing) {
            screen().drawingProxy().erase(true);
            screen().removeOverlay(this);
        }
        return true;

     default:
        // Swallow all keys other than movement
        // FIXME: do we still need this? PCC/PCC2 needs this for internal consistency,
        // but our DeleteDrawingOverlay is pretty robust against things happening in parallel.
        switch (key & ~util::KeyMod_Mask) {
         case util::Key_Left:
         case util::Key_Right:
         case util::Key_Up:
         case util::Key_Down:
         case util::Key_WheelUp:
         case util::Key_WheelDown:
            return false;

         default:
            return true;
        }
    }
}

bool
client::map::DeleteDrawingOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::DeleteDrawingOverlay::onTimer()
{
    m_phase = !m_phase;
    m_timer->setInterval(ui::CURSOR_BLINK_INTERVAL);
    requestRedraw();
}
