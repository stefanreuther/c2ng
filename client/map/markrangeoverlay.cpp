/**
  *  \file client/map/markrangeoverlay.cpp
  *  \brief Class client::map::MarkRangeOverlay
  */

#include "client/map/markrangeoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/map/screen.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

client::map::MarkRangeOverlay::MarkRangeOverlay(ui::Root& root, afl::string::Translator& tx, Location& loc, Screen& screen)
    : m_root(root),
      m_translator(tx),
      m_location(loc),
      m_screen(screen),
      m_origin(loc.getPosition()),
      m_end(loc.getPosition()),
      m_proxy(screen.gameSender(), root.engine().dispatcher()),
      m_numObjectsInRange(),
      conn_positionChange(m_location.sig_positionChange.add(this, &MarkRangeOverlay::onPositionChange)),
      conn_numObjectsInRange(m_proxy.sig_numObjectsInRange.add(this, &MarkRangeOverlay::onNumObjectsInRange))
{
    rebuildSelection();
}

void
client::map::MarkRangeOverlay::drawBefore(gfx::Canvas& can, const Renderer& ren)
{
    // ex WSelectChartMode::drawBelow
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.setColor(ui::Color_Shield+4);

    // Figure screen points for drawing
    gfx::Point originScaled = ren.scale(m_origin);
    gfx::Point endScaled = ren.scale(m_end);

    if (endScaled == originScaled) {
        // Single point
        drawPixel(ctx, originScaled);
    } else {
        // Area
        gfx::Rectangle r(originScaled, gfx::Point(1, 1));
        r.include(endScaled);
        drawRectangle(ctx, r);

        // Content of area if present
        if (r.getWidth() > 2 && r.getHeight() > 2) {
            r.grow(-1, -1);
            drawSolidBar(ctx, r, ui::Color_Shield + 3);
        }
    }
}

void
client::map::MarkRangeOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WSelectChartMode::drawOverlays
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    const int lineHeight = font->getLineHeight();

    gfx::Point pos = ren.getExtent().getTopLeft();

    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.setColor(ui::Color_White);
    ctx.useFont(*font);

    outText(ctx, pos, m_translator("Mark range, end with [ESC], cancel with [Backspace]."));

    if (m_numObjectsInRange != 0) {
        pos.addY(lineHeight);
        outText(ctx, pos, afl::string::Format(m_translator("%d unit%!1{s%} in range."), m_numObjectsInRange));
    }
}

bool
client::map::MarkRangeOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::MarkRangeOverlay::handleKey(util::Key_t key, int /*prefix*/, const Renderer& /*ren*/)
{
    // ex WSelectChartMode::handleEvent
    // FIXME: what about selection switching? PCC 1.x handles this explicitly
    // for Alt-Left/Right, but doesn't handle Alt-. or '.' specially.
    switch (key) {
     case util::Key_Escape:
     case 'r':
     case util::Key_Quit:
        // Exit mode
        m_screen.removeOverlay(this);
        return true;

     case util::Key_Backspace:
        // Cancel
        m_proxy.revertCurrentLayer();
        m_location.setPosition(m_origin);
        m_screen.removeOverlay(this);
        return true;

     default:
        return false;
    }
}

bool
client::map::MarkRangeOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::MarkRangeOverlay::onPositionChange(game::map::Point pt)
{
    // ex WSelectChartMode::onMove
    // FIXME: handle wrap: origin += loc.getWrapAdjust();
    if (m_end != pt) {
        m_end = pt;
        rebuildSelection();
    }
}

void
client::map::MarkRangeOverlay::onNumObjectsInRange(int n)
{
    if (m_numObjectsInRange != n) {
        m_numObjectsInRange = n;
        requestRedraw();
    }
}

void
client::map::MarkRangeOverlay::rebuildSelection()
{
    m_proxy.markObjectsInRange(m_origin, m_end, true);
}
