/**
  *  \file client/map/starchartoverlay.cpp
  */

#include "client/map/starchartoverlay.hpp"
#include "client/map/callback.hpp"
#include "client/map/location.hpp"
#include "client/map/renderer.hpp"
#include "client/map/screen.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

using client::proxy::LockProxy;

namespace {
    int determineDistance(util::Key_t key, int prefix)
    {
        // ex CC$ScannerMoveX, CC$ScannerMoveY
        // Movement is implemented as script in PCC2, but native in c2ng for greater fluency.
        return prefix != 0
            ? prefix
            : (key & util::KeyMod_Shift) != 0
            ? 1
            : (key & util::KeyMod_Ctrl) != 0
            ? 100
            : 10;
    }
}



client::map::StarchartOverlay::StarchartOverlay(ui::Root& root, Location& loc, Screen& scr,
                                                util::RequestSender<game::Session> gameSender)
    : m_root(root),
      m_location(loc),
      m_screen(scr),
      m_lockProxy(root.engine().dispatcher(), gameSender),
      conn_objectChange(loc.sig_objectChange.add(this, &StarchartOverlay::onChange)),
      conn_positionChange(loc.sig_positionChange.add(this, &StarchartOverlay::onChange))
{
    m_lockProxy.sig_result.add(this, &StarchartOverlay::onLockResult);
}

// Overlay:
void
client::map::StarchartOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
}

void
client::map::StarchartOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WStandardChartMode::drawOverlays
    // FIXME: should filter for !PrimaryLayer?
    m_screen.drawObjectList(can);
    m_screen.drawTiles(can);

    // Coordinates
    game::map::Point pt = m_location.configuration().getSimpleCanonicalLocation(m_location.getPosition());
    game::map::Point pt1 = m_location.configuration().getCanonicalLocation(pt);

    gfx::Rectangle area = ren.getExtent();

    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().setStyle(ui::FixedFont));
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*font);
    ctx.setColor(ui::Color_White);
    ctx.setTextAlign(2, 2);
    if (pt == pt1) {
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY()), afl::string::Format("%4d,%4d", pt.getX(), pt.getY()));
    } else {
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY() - font->getLineHeight()), afl::string::Format("%4d,%4d", pt.getX(), pt.getY()));
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY()), afl::string::Format(_("wraps to %4d,%4d"), pt1.getX(), pt1.getY()));
    }

    // Sector Number
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    int sectorNumber = m_location.configuration().getSectorNumber(pt);
    if (sectorNumber != 0) {
        ctx.setTextAlign(0, 2);
        outText(ctx, gfx::Point(area.getLeftX(), area.getBottomY()), afl::string::Format(_("Sector %d"), sectorNumber));
    }

    // FIXME: Filter
    // atom_t f = loc.getViewport().drawing_filter;
    // if (f != GChartViewport::NO_FILTER) {
    //     ctx.setTextAlign(0, 0);
    //     outText(ctx, view.x, view.y, format(_("Drawing filter: showing only %s"),
    //                                         isAtom(f) ? atomStr(f) : itoa(f)));
    // }
}

bool
client::map::StarchartOverlay::drawCursor(gfx::Canvas& can, const Renderer& ren)
{
    // ex WStandardChartMode::drawCursor
    gfx::Point sc = ren.scale(m_location.configuration().getSimpleNearestAlias(m_location.getPosition(), ren.getCenter()));

    if (!m_location.getFocusedObject().isSet()) {
        gfx::Context<uint8_t> ctx(can, m_screen.root().colorScheme());
        ctx.setColor(ui::Color_Blue);
        drawHLine(ctx, sc.getX()-30, sc.getY(), sc.getX()-6);
        drawHLine(ctx, sc.getX()+30, sc.getY(), sc.getX()+6);
        drawVLine(ctx, sc.getX(), sc.getY()-30, sc.getY()-6);
        drawVLine(ctx, sc.getX(), sc.getY()+30, sc.getY()+6);
    } else {
    // FIXME    ::drawCursor(can, sc, cursor_angle);
    }

    return true;
}

// EventConsumer:
bool
client::map::StarchartOverlay::handleKey(util::Key_t key, int prefix, const Renderer& /*ren*/)
{
    // ex WStandardChartMode::handleEvent
    switch (key) {
     case util::Key_Left:
     case util::Key_Left + util::KeyMod_Shift:
     case util::Key_Left + util::KeyMod_Ctrl:
        m_location.moveRelative(-determineDistance(key, prefix), 0);
        return true;

     case util::Key_Right:
     case util::Key_Right + util::KeyMod_Shift:
     case util::Key_Right + util::KeyMod_Ctrl:
        m_location.moveRelative(determineDistance(key, prefix), 0);
        return true;

     case util::Key_Up:
     case util::Key_Up + util::KeyMod_Shift:
     case util::Key_Up + util::KeyMod_Ctrl:
        m_location.moveRelative(0, determineDistance(key, prefix));
        return true;

     case util::Key_Down:
     case util::Key_Down + util::KeyMod_Shift:
     case util::Key_Down + util::KeyMod_Ctrl:
        m_location.moveRelative(0, -determineDistance(key, prefix));
        return true;

     case util::Key_Tab:
     case util::Key_Tab + util::KeyMod_Shift:
     case util::Key_Tab + util::KeyMod_Ctrl:
     case util::Key_Tab + util::KeyMod_Ctrl + util::KeyMod_Shift:
        m_location.cycleFocusedObject((key & util::KeyMod_Shift) == 0,
                                      (key & util::KeyMod_Ctrl) != 0);
        return true;

     case util::Key_Return:
     case util::Key_Return + util::KeyMod_Ctrl:
     case ' ':
     case ' ' + util::KeyMod_Ctrl:
        // FIXME: this means the locked object will flicker if users repeatedly press Enter; that does not happen in PCC2.
        // We could avoid that by pre-validating the object list whether it already matches our desired object.
        if (m_location.startJump()) {
            LockProxy::Flags_t flags;
            if ((key & util::Key_Mask) == ' ') {
                flags += LockProxy::Left;
            }
            if ((key & util::KeyMod_Ctrl) != 0) {
                flags += LockProxy::MarkedOnly;
            }
            m_lockProxy.postQuery(m_location.getPosition(), flags);
        }
        return true;
    }
    return false;
}

bool
client::map::StarchartOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::StarchartOverlay::onChange()
{
    if (Callback* pCB = getCallback()) {
        pCB->requestRedraw();
    }
}

void
client::map::StarchartOverlay::onLockResult(game::map::Point pt)
{
    m_location.setPosition(pt);
}
