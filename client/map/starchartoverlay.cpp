/**
  *  \file client/map/starchartoverlay.cpp
  */

#include <cmath>
#include "client/map/starchartoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/dialogs/newdrawingtag.hpp"
#include "client/downlink.hpp"
#include "client/map/callback.hpp"
#include "client/map/deletedrawingoverlay.hpp"
#include "client/map/distanceoverlay.hpp"
#include "client/map/drawcircleoverlay.hpp"
#include "client/map/drawlineoverlay.hpp"
#include "client/map/location.hpp"
#include "client/map/markeroverlaybase.hpp"
#include "client/map/markrangeoverlay.hpp"
#include "client/map/movemarkeroverlay.hpp"
#include "client/map/renderer.hpp"
#include "client/map/screen.hpp"
#include "client/widgets/helpwidget.hpp"
#include "client/widgets/markercolorselector.hpp"
#include "client/widgets/markerkindselector.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/icons/balloon.hpp"
#include "ui/icons/colortext.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/skincolorscheme.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/scrollbarcontainer.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/window.hpp"
#include "util/translation.hpp"

using game::proxy::LockProxy;

namespace {
    /* What distance is considered "near" for drawings? */
    const int NEAR_DISTANCE = 21;

    /* Determine distance-to-move from key and prefix argument */
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


    /*
     *  Cursor oscillation
     */

    const int PHASE_MAX = 80;
    const int PHASE_REPEAT = 50;

    int getDeltaFromPhase(int phase)
    {
        if (phase < 20) {
            // One falling edge (half period), 20 ticks, amplitude 40
            return int(0.5 + 20 + 20*std::cos(phase * 3.141592/20));
        } else if (phase < 50) {
            // One entire period (rising+falling), 30 ticks, amplitude 5
            return int(0.5 + 5 - 5*std::cos((phase-20) * 3.141592/15));
        } else {
            // One entire period (rising+falling), 30 ticks, amplitude 2
            return int(0.5 + 2 - 2*std::cos((phase-50) * 3.141592/15));
        }
    }
}



client::map::StarchartOverlay::StarchartOverlay(ui::Root& root, afl::string::Translator& tx, Location& loc, Screen& scr)
    : m_root(root),
      m_translator(tx),
      m_location(loc),
      m_screen(scr),
      m_drawingTagFilterActive(false),
      m_drawingTagFilter(),
      m_drawingTagFilterName(),
      m_cursorPosition(),
      m_cursorArea(),
      m_cursorPhase(),
      conn_objectChange(loc.sig_objectChange.add(this, &StarchartOverlay::onChange)),
      conn_positionChange(loc.sig_positionChange.add(this, &StarchartOverlay::onChange)),
      conn_effectTimer(scr.sig_effectTimer.add(this, &StarchartOverlay::onEffectTimer))
{ }

// Overlay:
void
client::map::StarchartOverlay::drawBefore(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
}

void
client::map::StarchartOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WStandardChartMode::drawOverlays
    // Coordinates
    game::map::Point pt = m_location.configuration().getSimpleCanonicalLocation(m_location.getPosition());
    game::map::Point pt1 = m_location.configuration().getCanonicalLocation(pt);

    gfx::Rectangle area = ren.getExtent();

    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().setStyle(ui::FixedFont));
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*font);
    ctx.setColor(ui::Color_White);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    if (pt == pt1) {
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY()), afl::string::Format("%4d,%4d", pt.getX(), pt.getY()));
    } else {
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY() - font->getLineHeight()), afl::string::Format("%4d,%4d", pt.getX(), pt.getY()));
        outText(ctx, gfx::Point(area.getRightX(), area.getBottomY()), afl::string::Format(m_translator("wraps to %4d,%4d"), pt1.getX(), pt1.getY()));
    }

    // Sector Number
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    int sectorNumber = m_location.configuration().getSectorNumber(pt);
    if (sectorNumber != 0) {
        ctx.setTextAlign(gfx::LeftAlign, gfx::BottomAlign);
        outText(ctx, gfx::Point(area.getLeftX(), area.getBottomY()), afl::string::Format(m_translator("Sector %d"), sectorNumber));
    }

    // Filter
    if (m_drawingTagFilterActive) {
        ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
        outText(ctx, area.getTopLeft(), afl::string::Format(m_translator("Drawing filter: showing only %s"), m_drawingTagFilterName));
    }
}

bool
client::map::StarchartOverlay::drawCursor(gfx::Canvas& can, const Renderer& ren)
{
    // ex WStandardChartMode::drawCursor
    gfx::Point sc = ren.scale(m_location.configuration().getSimpleNearestAlias(m_location.getPosition(), ren.getCenter()));

    if (!m_location.getFocusedObject().isSet()) {
        // Nothing focused: draw cross
        gfx::Context<uint8_t> ctx(can, m_screen.root().colorScheme());
        ctx.setColor(ui::Color_Blue);
        drawHLine(ctx, sc.getX()-30, sc.getY(), sc.getX()-6);
        drawHLine(ctx, sc.getX()+30, sc.getY(), sc.getX()+6);
        drawVLine(ctx, sc.getX(), sc.getY()-30, sc.getY()-6);
        drawVLine(ctx, sc.getX(), sc.getY()+30, sc.getY()+6);
    } else {
        // Object is focused: draw wobbly label
        // First, determine label and skin color
        String_t label;
        util::SkinColor::Color color = util::SkinColor::Static;
        if (const game::ref::UserList::Item* it = m_location.getObjectByIndex(m_location.getCurrentObjectIndex())) {
            color = it->color;
            switch (it->reference.getType()) {
             case game::Reference::Ship:
                label = afl::string::Format(m_translator("Ship #%d"), it->reference.getId());
                break;
             case game::Reference::Planet:
                label = afl::string::Format(m_translator("Planet #%d"), it->reference.getId());
                break;
             default:
                break;
            }
        }

        if (!label.empty()) {
            // Determine icon color from skin color
            uint8_t textColor = ui::Color_Grayscale + 12;
            uint8_t frameColor = ui::Color_Grayscale + 6;
            switch (color) {
             case util::SkinColor::Green:  textColor = ui::Color_Green;  frameColor = ui::Color_GreenScale + 6;      break;
             case util::SkinColor::Red:    textColor = ui::Color_Red;    frameColor = ui::Color_Fire + 6;            break;
             case util::SkinColor::Yellow: textColor = ui::Color_Yellow; frameColor = ui::Color_DarkYellowScale + 6; break;
             default:                                                                                                break;
            }

            // Icons to draw
            ui::icons::ColorText text(label, m_root);
            text.setColor(textColor);
            ui::icons::Balloon frame(text, m_root, frameColor);

            // Determine position
            const int delta = getDeltaFromPhase(m_cursorPhase);
            const int x = sc.getX();
            const int y = sc.getY() - delta - 6;
            gfx::Point size = frame.getSize();
            gfx::Rectangle area(x - size.getX() / 2, y - size.getY(), size.getX(), size.getY());

            // Draw
            ui::SkinColorScheme scheme(ui::GRAY_COLOR_SET, m_root.colorScheme());
            gfx::Context<util::SkinColor::Color> ctx(can, scheme);
            frame.draw(ctx, area, ui::ButtonFlags_t());

            m_cursorArea = area;
            m_cursorArea.grow(10, 10);
        }
    }

    // Draw tiles after cursor so they appear above it
    // FIXME: should filter for !PrimaryLayer?
    m_screen.drawObjectList(can);
    m_screen.drawTiles(can);

    return true;
}

// EventConsumer:
bool
client::map::StarchartOverlay::handleKey(util::Key_t key, int prefix, const Renderer& /*ren*/)
{
    // ex WStandardChartMode::handleEvent
    // Keymap keys override default keys, but not other modes' keys.
    if (m_screen.handleKeymapKey(key, prefix)) {
        return true;
    }

    // FIXME: remapWheelKey (PCC 2.0.12, #397)
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
     case ' ' + util::KeyMod_Ctrl: {
        LockProxy::Flags_t flags;
        if ((key & util::Key_Mask) == ' ') {
            flags += LockProxy::Left;
        }
        if ((key & util::KeyMod_Ctrl) != 0) {
            flags += LockProxy::MarkedOnly;
        }
        m_screen.lockObject(flags);
        return true;
     }

     case util::Key_PgDn:
     case util::Key_PgDn + util::KeyMod_Ctrl:
     case util::Key_PgDn + util::KeyMod_Shift:
     case util::Key_PgDn + util::KeyMod_Shift + util::KeyMod_Ctrl:
     case util::Key_PgUp:
     case util::Key_PgUp + util::KeyMod_Ctrl:
     case util::Key_PgUp + util::KeyMod_Shift:
     case util::Key_PgUp + util::KeyMod_Shift + util::KeyMod_Ctrl: {
        game::map::Location::BrowseFlags_t flags;
        if ((key & util::Key_Mask) == util::Key_PgUp) {
            flags += game::map::Location::Backwards;
        }
        if ((key & util::KeyMod_Ctrl) != 0) {
            flags += game::map::Location::MarkedOnly;
        }
        if ((key & util::KeyMod_Shift) == 0) {
            flags += game::map::Location::PlayedOnly;
        }
        m_screen.browse(flags);
        return true;
     }

     case '+':
        m_screen.mapWidget().zoomIn();
        return true;

     case '-':
        m_screen.mapWidget().zoomOut();
        return true;

     case 'c':
        editMarkerColor();
        return true;

     case 'd':
        startDistance();
        return true;

     case 'f' + util::KeyMod_Alt:
        editDrawingTagFilter();
        return true;

     case 'p':
        startDrawing();
        return true;

     case 'r':
        startMarkRange();
        return true;

     case 't':
        editMarkerTag();
        return true;

     case 'v':
        startMovingMarker();
        return true;

     case 'x':
        moveInsideOut();
        return true;

     case util::Key_Delete:
        startDeleting();
        return true;

     case util::Key_F9:
     case util::Key_F9 + util::KeyMod_Alt:
        editMarkerComment();
        return true;

     default:
        if ((key & util::KeyMod_Alt) != 0) {
            game::map::RenderOptions::Options_t opts = game::map::RenderOptions::getOptionFromKey(key & ~(util::KeyMod_Alt | util::KeyMod_Ctrl));
            if (!opts.empty()) {
                m_screen.mapWidget().toggleOptions(opts);
                return true;
            }
        }
        break;
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
    if (m_cursorPosition != m_location.getPosition()) {
        m_cursorPosition = m_location.getPosition();
        m_cursorPhase = 0;
    }
    requestRedraw();
}

void
client::map::StarchartOverlay::onEffectTimer()
{
    if (m_location.getFocusedObject().isSet()) {
        ++m_cursorPhase;
        if (m_cursorPhase >= PHASE_MAX) {
            m_cursorPhase = PHASE_REPEAT;
        }

        // Instead of invalidating the entire frame, invalidate only the (estimated) area affected by cursor redraw.
        // This estimate is provided by the last draw() operation.
        // As of 20220427, this reduces CPU usage from 36% -> 28% on my machine on a 1920x1080 starchart.
        // Potential error: if we miss some frames, the estimate produced by previous executed draw
        // may not cover the correct area; this causes a glitch that fixes itself, so probably not worth bothering with.
        if (Callback* cb = getCallback()) {
            cb->requestRedraw(m_cursorArea);
        }
    }
}

void
client::map::StarchartOverlay::editDrawingTagFilter()
{
    if (m_drawingTagFilterActive) {
        // Active -> Inactive
        clearDrawingTagFilter();
    } else {
        // ex selectMarkerTag
        // Ask for new: get list
        util::StringList tagList;
        Downlink link(m_root, m_translator);
        m_screen.drawingProxy().getTagList(link, tagList);
        if (tagList.empty()) {
            return;
        }

        // Dialog
        afl::base::Deleter del;
        ui::Window& win = del.addNew(new ui::Window(m_translator("Filter Drawings"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5));
        ui::widgets::StringListbox& box = del.addNew(new ui::widgets::StringListbox(m_root.provider(), m_root.colorScheme()));
        box.setPreferredHeight(20);
        box.setPreferredWidth(30, false);
        box.swapItems(tagList);

        win.add(del.addNew(new ui::widgets::ScrollbarContainer(box, m_root)));

        client::widgets::HelpWidget& help = del.addNew(new client::widgets::HelpWidget(m_root, m_translator, m_screen.gameSender(), "pcc2:drawtag"));
        ui::widgets::StandardDialogButtons& btn = del.addNew(new ui::widgets::StandardDialogButtons(m_root, m_translator));
        ui::EventLoop loop(m_root);
        btn.addStop(loop);
        btn.addHelp(help);

        win.add(btn);
        win.add(help);
        win.add(del.addNew(new ui::widgets::Quit(m_root, loop)));
        win.pack();
        m_root.centerWidget(win);
        m_root.add(win);
        if (loop.run()) {
            int32_t k;
            String_t v;
            if (box.getStringList().get(box.getCurrentItem(), k, v)) {
                setDrawingTagFilter(static_cast<util::Atom_t>(k), v);
            }
        }
    }
}

void
client::map::StarchartOverlay::editMarkerColor()
{
    // ex client/chart/standardmode.cc:doColorChange
    // Make a local proxy to not interfere with a possible active mode
    game::proxy::DrawingProxy proxy(m_screen.gameSender(), m_root.engine().dispatcher());
    Downlink link(m_root, m_translator);

    // FIXME -> port this // Not possible if drawings not visible
    // if ((getChartOpts(false, viewport.mult, viewport.divi).show & GChartOptions::co_Drawings) == 0)
    //     return;

    // Find nearest visible drawing
    proxy.selectNearestVisibleDrawing(m_location.getPosition(), NEAR_DISTANCE);
    game::proxy::DrawingProxy::Status_t st;
    proxy.getStatus(link, st);
    const game::map::Drawing* p = st.get();
    if (p == 0) {
        return;
    }

    // Change color
    client::widgets::MarkerColorSelector csel(m_root);
    csel.setColor(p->getColor());

    bool adjacent = false;
    bool ok = csel.doStandardDialog(m_translator("Drawing Color"), m_translator, (p->getType() == game::map::Drawing::LineDrawing ? &adjacent : 0));
    if (!ok) {
        return;
    }

    // Update
    proxy.setColor(csel.getColor(), adjacent);
    proxy.finish();
}

void
client::map::StarchartOverlay::startDrawing()
{
    // ex doDraw
    // Ask user what to do
    client::dialogs::NewDrawingInfo info;
    if (!chooseNewDrawingParameters(info, m_root, m_screen.gameSender(), m_translator)) {
        return;
    }

    // Draw it
    game::proxy::DrawingProxy::Status_t st;
    Downlink link(m_root, m_translator);
    switch (info.type) {
     case game::map::Drawing::LineDrawing:
     case game::map::Drawing::RectangleDrawing:
        m_screen.drawingProxy().create(m_location.getPosition(), info.type);
        m_screen.drawingProxy().setColor(info.color, false);
        m_screen.drawingProxy().setTagName(info.tagName, false);
        m_screen.drawingProxy().getStatus(link, st);
        if (const game::map::Drawing* p = st.get()) {
            m_screen.setNewOverlay(Screen::PrimaryLayer, new DrawLineOverlay(m_root, m_translator, m_location, m_screen, *p));
        }
        break;

     case game::map::Drawing::MarkerDrawing:
        createMarker(info);
        break;

     case game::map::Drawing::CircleDrawing:
        m_screen.drawingProxy().create(m_location.getPosition(), info.type);
        m_screen.drawingProxy().setColor(info.color, false);
        m_screen.drawingProxy().setTagName(info.tagName, false);
        m_screen.drawingProxy().getStatus(link, st);
        if (const game::map::Drawing* p = st.get()) {
            m_screen.setNewOverlay(Screen::PrimaryLayer, new DrawCircleOverlay(m_root, m_translator, m_location, m_screen, *p));
        }
        break;
    }
}

void
client::map::StarchartOverlay::createMarker(const client::dialogs::NewDrawingInfo& info)
{
    const String_t title = m_translator("New marker");
    client::widgets::MarkerColorSelector mcs(m_root);
    client::widgets::MarkerKindSelector mks(m_root);
    mcs.setColor(info.color);

    if (mks.doStandardDialog(title, m_translator)) {
        if (mcs.doStandardDialog(title, m_translator, 0)) {
            game::proxy::DrawingProxy& proxy = m_screen.drawingProxy();
            proxy.create(m_location.getPosition(), info.type);
            proxy.setColor(mcs.getColor(), false);
            proxy.setMarkerKind(mks.getMarkerKind());
            proxy.setTagName(info.tagName, false);
            proxy.finish();
        }
    }
}

void
client::map::StarchartOverlay::startMarkRange()
{
    // ex doMarkRange
    m_screen.setNewOverlay(Screen::PrimaryLayer, new MarkRangeOverlay(m_root, m_translator, m_location, m_screen));
}

void
client::map::StarchartOverlay::editMarkerTag()
{
    // ex doTagChange
    // FIXME: // Not possible if drawings not visible
    // if ((getChartOpts(false, viewport.mult, viewport.divi).show & GChartOptions::co_Drawings) == 0)
    //     return;

    // Find nearest visible drawing
    game::proxy::DrawingProxy& proxy = m_screen.drawingProxy();
    Downlink link(m_root, m_translator);
    proxy.selectNearestVisibleDrawing(m_location.getPosition(), NEAR_DISTANCE);
    game::proxy::DrawingProxy::Status_t st;
    proxy.getStatus(link, st);
    const game::map::Drawing* p = st.get();
    if (p == 0) {
        return;
    }

    // Fetch list of tags
    util::StringList tagList;
    proxy.getTagList(link, tagList);
    tagList.sortAlphabetically();

    // Dialog
    client::dialogs::NewDrawingTag dlg(tagList, m_root, m_screen.gameSender());
    dlg.setTag(p->getTag());

    bool adjacent = false;
    if (dlg.run(m_translator("Drawing Tag"), m_translator, &adjacent)) {
        proxy.setTagName(dlg.getTagName(), adjacent);
    }
    proxy.finish();
}

void
client::map::StarchartOverlay::editMarkerComment()
{
    // ex tryEditMarkerComment
    game::proxy::DrawingProxy& proxy = m_screen.drawingProxy();
    Downlink link(m_root, m_translator);
    proxy.selectMarkerAt(m_location.getPosition());
    game::proxy::DrawingProxy::Status_t st;
    proxy.getStatus(link, st);
    const game::map::Drawing* p = st.get();
    if (p != 0) {
        client::map::editMarkerComment(m_root, *p, proxy, m_translator);
    }
}

void
client::map::StarchartOverlay::startMovingMarker()
{
    game::proxy::DrawingProxy& proxy = m_screen.drawingProxy();
    Downlink link(m_root, m_translator);
    proxy.selectMarkerAt(m_location.getPosition());
    game::proxy::DrawingProxy::Status_t st;
    proxy.getStatus(link, st);
    const game::map::Drawing* p = st.get();
    if (p != 0) {
        m_screen.setNewOverlay(Screen::PrimaryLayer, new MoveMarkerOverlay(m_root, m_translator, m_location, m_screen, *p));
    }
}

void
client::map::StarchartOverlay::startDeleting()
{
    // ex doDelete
    // FIXME: // Not possible if drawings not visible
    // if ((getChartOpts(false, viewport.mult, viewport.divi).show & GChartOptions::co_Drawings) == 0)
    //     return;

    // Find nearest visible drawing
    game::proxy::DrawingProxy& proxy = m_screen.drawingProxy();
    Downlink link(m_root, m_translator);
    proxy.selectNearestVisibleDrawing(m_location.getPosition(), NEAR_DISTANCE);
    game::proxy::DrawingProxy::Status_t st;
    proxy.getStatus(link, st);
    const game::map::Drawing* p = st.get();
    if (p != 0) {
        m_screen.setNewOverlay(Screen::PrimaryLayer, new DeleteDrawingOverlay(m_root, m_translator, m_screen, *p));
    }
}

void
client::map::StarchartOverlay::startDistance()
{
    // ex doDistance
    // Determine current ship
    game::Id_t shipId = 0;
    game::Reference currentObject = m_location.getFocusedObject();
    if (currentObject.getType() == game::Reference::Ship) {
        shipId = currentObject.getId();
    }

    // Add mode
    m_screen.setNewOverlay(Screen::PrimaryLayer, new DistanceOverlay(m_screen, m_location, m_location.getPosition(), shipId));
}

void
client::map::StarchartOverlay::moveInsideOut()
{
    const game::map::Configuration& config = m_location.configuration();
    if (config.getMode() == game::map::Configuration::Circular) {
        // Determine location to jump to
        bool ok;
        game::map::Point pt = config.getCanonicalLocation(m_location.getPosition());
        if (pt != m_location.getPosition()) {
            // Move outside-in
            ok = true;
        } else {
            // Try to move inside-out
            game::map::Point pt2;
            if (config.getPointAlias(pt, pt2, 1, true)) {
                pt = pt2;
                ok = true;
            } else {
                ok = false;
            }
        }

        // Execute the jump
        if (ok) {
            if (m_location.startJump()) {
                m_location.setPosition(pt);
            }
        }
    }
}

void
client::map::StarchartOverlay::setDrawingTagFilter(util::Atom_t tag, String_t tagName)
{
    if (!m_drawingTagFilterActive || m_drawingTagFilter != tag) {
        // FIXME: configure locking, selectNearestVisibleDrawing
        m_drawingTagFilterActive = true;
        m_drawingTagFilter = tag;
        m_drawingTagFilterName = tagName;
        m_screen.mapWidget().setDrawingTagFilter(tag);
        requestRedraw();
    }
}

void
client::map::StarchartOverlay::clearDrawingTagFilter()
{
    if (m_drawingTagFilterActive) {
        // FIXME: configure locking, selectNearestVisibleDrawing
        m_drawingTagFilterActive = false;
        m_screen.mapWidget().clearDrawingTagFilter();
        requestRedraw();
    }
}
