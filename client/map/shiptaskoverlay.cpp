/**
  *  \file client/map/shiptaskoverlay.cpp
  *  \brief Class client::map::ShipTaskOverlay
  */

#include <cmath>
#include "client/map/shiptaskoverlay.hpp"
#include "afl/string/format.hpp"
#include "client/map/renderer.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

client::map::ShipTaskOverlay::ShipTaskOverlay(ui::Root& root)
    : m_root(root),
      m_status()
{ }

client::map::ShipTaskOverlay::~ShipTaskOverlay()
{ }

void
client::map::ShipTaskOverlay::setStatus(const game::proxy::TaskEditorProxy::ShipStatus& status)
{
    m_status = status;
    requestRedraw();
}

void
client::map::ShipTaskOverlay::drawBefore(gfx::Canvas& can, const Renderer& ren)
{
    // ex WShipTaskScannerChartWidget::drawPre
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());

    gfx::Point pt = ren.scale(m_status.startPosition);
    for (size_t i = 0, n = m_status.positions.size(); i < n; ++i) {
        uint8_t pix;
        if (i < m_status.numFuelPositions) {
            ctx.setColor(ui::Color_DarkYellowScale+6);
            ctx.setLinePattern(gfx::SOLID_LINE);
            pix = ui::Color_Yellow;
        } else {
            ctx.setColor(ui::Color_Dark);
            ctx.setLinePattern(gfx::DOTTED_LINE);
            pix = ui::Color_Gray;
        }

        gfx::Point npt = ren.scale(m_status.positions[i]);
        drawLine(ctx, pt, npt);

        ctx.setColor(pix);
        drawPixel(ctx, npt + gfx::Point(1, 0));
        drawPixel(ctx, npt + gfx::Point(-1, 0));
        drawPixel(ctx, npt + gfx::Point(0, 1));
        drawPixel(ctx, npt + gfx::Point(0, -1));
        pt = npt;
    }
}

void
client::map::ShipTaskOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WShipTaskScannerChartWidget::drawPost(GfxCanvas& can)
    // FIXME: display of distances is optional (static variable in PCC2, should be preferences option
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont("-"));
    gfx::Point pt = ren.scale(m_status.startPosition);
    for (size_t i = 0, n = std::min(m_status.positions.size(), m_status.distances2.size()); i < n; ++i) {
        if (i < m_status.numFuelPositions) {
            ctx.setColor(ui::Color_DarkYellow);
        } else {
            ctx.setColor(ui::Color_Gray);
        }

        /* Draw only segments of more than 10 ly to avoid cluttering up display too badly. */
        long d2 = m_status.distances2[i];
        gfx::Point npt = ren.scale(m_status.positions[i]);
        if (d2 > 100) {
            /* Place distances atop each segment.
               If the segment is falling, draw to the right [\''],
               if the segment is rising, draw to the left [''/]. */
            if ((npt.getY() < pt.getY()) != (npt.getX() < pt.getX())) {
                ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
            } else {
                ctx.setTextAlign(gfx::LeftAlign, gfx::BottomAlign);
            }
            outText(ctx, gfx::Point((npt.getX() + pt.getX())/2, (npt.getY() + pt.getY())/2), afl::string::Format("%.0f ly", std::sqrt(double(d2))));
        }
        pt = npt;
    }
}

bool
client::map::ShipTaskOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::ShipTaskOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::ShipTaskOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}
