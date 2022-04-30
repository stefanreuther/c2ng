/**
  *  \file client/map/movemarkeroverlay.cpp
  */

#include "client/map/movemarkeroverlay.hpp"
#include "client/map/location.hpp"
#include "client/map/screen.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/widgets/inputline.hpp"

// FIXME: port this? Ignore current marker for locking purposes.
// void
// WMoveMarkerChartMode::doChartLock(GChartLocation& loc, TLockMode lm, bool marked)
// {
//     GPoint pt = loc.getLocation();
//     GLockData locker(pt, marked);
//     if (*drawing) {
//         locker.setIgnore(*drawing);
//     }
//     findItem(getDisplayedTurn().getCurrentUniverse(), locker, lm);
//     if (const GMapObject* obj = locker.getObject()) {
//         loc.setCurrentObject(*obj);
//     }
//     loc.setLocation(locker.getPoint().getSimpleNearestAlias(loc.getLocation()));
// }


client::map::MoveMarkerOverlay::MoveMarkerOverlay(ui::Root& root,
                                                  afl::string::Translator& tx,
                                                  client::map::Location& loc,
                                                  client::map::Screen& screen,
                                                  const game::map::Drawing& drawing)
    : MarkerOverlayBase(root, tx, screen, drawing),
      m_location(loc),
      conn_positionChange(loc.sig_positionChange.add(this, &MoveMarkerOverlay::onPositionChange))
{ }

void
client::map::MoveMarkerOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{ }

void
client::map::MoveMarkerOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WMoveMarkerChartMode::drawOverlays
    afl::base::Ref<gfx::Font> font = root().provider().getFont(gfx::FontRequest());
    String_t text = translator()("Move marker, end with [ESC].");

    const int width = font->getTextWidth(text) + 20;
    const int height = font->getTextHeight(text);

    gfx::Point center = ren.getExtent().getCenter();
    gfx::Rectangle area(center.getX() - width/2, center.getY() + height*5/4, width, height);

    gfx::Context<uint8_t> ctx(can, root().colorScheme());
    ctx.useFont(*font);
    ctx.setColor(ui::Color_White);

    drawSolidBar(ctx, area, ui::Color_Red);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    outTextF(ctx, area, text);
}

bool
client::map::MoveMarkerOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MoveMarkerOverlay::handleKey(util::Key_t key, int prefix, const Renderer& ren)
{
    // ex WMoveMarkerChartMode::handleEvent
    switch (key) {
     case 'v':
        // Exit move mode
        screen().removeOverlay(this);
        return true;

     case util::Key_F9:
     case util::Key_F9 + util::KeyMod_Alt:
        // Change comment
        editComment();
        return true;

        // FIXME: custom locking that ignores this marker
        // case SDLK_RETURN:
        //    // Locking; mostly copied from standardmode
        //    doChartLock(loc, lm_Right, false);
        //    return true;
        // case SDLK_SPACE:
        //    doChartLock(loc, lm_Left, false);
        //    return true;
        // case ss_Ctrl + SDLK_RETURN:
        //    doChartLock(loc, lm_Right, true);
        //    return true;
        // case ss_Ctrl + SDLK_SPACE:
        //    doChartLock(loc, lm_Left, true);
        //    return true;

     default:
        return defaultHandleKey(key, prefix, ren);
    }
}

bool
client::map::MoveMarkerOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::MoveMarkerOverlay::onPositionChange(game::map::Point pt)
{
    // ex WMoveMarkerChartMode::onMove
    screen().drawingProxy().setPos(pt);
}

void
client::map::MoveMarkerOverlay::editComment()
{
    editMarkerComment(root(), drawing(), screen().drawingProxy(), translator());
}


void
client::map::editMarkerComment(ui::Root& root,
                               const game::map::Drawing& marker,
                               game::proxy::DrawingProxy& proxy,
                               afl::string::Translator& tx)
{
    // ex editMarkerComment
    ui::widgets::InputLine input(255, root);
    input.setFlag(ui::widgets::InputLine::GameChars, true);
    input.setText(marker.getComment());
    if (input.doStandardDialog(tx("Marker Comment"), tx("Enter new comment for this marker:"), tx)) {
        proxy.setComment(input.getText());
    }
}
