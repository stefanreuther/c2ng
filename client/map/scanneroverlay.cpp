/**
  *  \file client/map/scanneroverlay.cpp
  */

#include "client/map/scanneroverlay.hpp"
#include "util/updater.hpp"
#include "client/map/callback.hpp"
#include "gfx/context.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"
#include "client/map/renderer.hpp"

namespace {
    void drawScannerEnd(gfx::BaseContext& ctx, gfx::Point pt, int size)
    {
        // ex GChartViewport::drawScannerEnd
        if (size >= 1) {
            drawVLine(ctx, pt.getX()-3, pt.getY()-2, pt.getY()+2);
            drawVLine(ctx, pt.getX()+3, pt.getY()-2, pt.getY()+2);
            drawHLine(ctx, pt.getX()-2, pt.getY()-3, pt.getX()+2);
            drawHLine(ctx, pt.getX()-2, pt.getY()+3, pt.getX()+2);
        } else {
            drawPixel(ctx, pt + gfx::Point(+1,  0));
            drawPixel(ctx, pt + gfx::Point(-1,  0));
            drawPixel(ctx, pt + gfx::Point( 0, +1));
            drawPixel(ctx, pt + gfx::Point( 0, -1));
        }
    }
}

client::map::ScannerOverlay::ScannerOverlay(ui::ColorScheme& cs)
    : m_colorScheme(cs), m_valid(false), m_origin(), m_target()
{ }

client::map::ScannerOverlay::~ScannerOverlay()
{ }

void
client::map::ScannerOverlay::drawBefore(gfx::Canvas& can, const Renderer& ren)
{
    // ex WScannerChartWidget::drawPre
    gfx::Context<uint8_t> ctx(can, m_colorScheme);
    gfx::Point m = ren.scale(m_origin);

    ctx.setColor(ui::Color_BlueBlack);

    drawHLine(ctx, m.getX() - 10, m.getY(),      m.getX() + 11);
    drawVLine(ctx, m.getX(),      m.getY() - 10, m.getY() + 11);
}

void
client::map::ScannerOverlay::drawAfter(gfx::Canvas& can, const Renderer& ren)
{
    // ex WScannerChartWidget::drawPost, WScannerChartWidget::drawScanner, GChartViewport::drawScanner
    if (m_valid) {
        gfx::Context<uint8_t> ctx(can, m_colorScheme);
        ctx.setColor(ui::Color_Yellow);

        gfx::Point aa = ren.scale(m_origin);
        gfx::Point bb = ren.scale(m_target);

        int size = ren.scale(1);
        drawScannerEnd(ctx, aa, size);
        if (aa != bb) {
            drawLine(ctx, aa, bb);
            drawScannerEnd(ctx, bb, size);
        }
    }
}

bool
client::map::ScannerOverlay::drawCursor(gfx::Canvas& /*can*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::ScannerOverlay::handleKey(util::Key_t /*key*/, int /*prefix*/, const Renderer& /*ren*/)
{
    return false;
}

bool
client::map::ScannerOverlay::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/, const Renderer& /*ren*/)
{
    return false;
}

void
client::map::ScannerOverlay::setPositions(game::map::Point origin, game::map::Point target)
{
    util::Updater u;
    u.set(m_origin, origin);
    u.set(m_target, target);
    u.set(m_valid, true);
    if (u) {
        requestRedraw();
    }
}

void
client::map::ScannerOverlay::clearPositions()
{
    util::Updater u;
    u.set(m_valid, false);
    if (u) {
        requestRedraw();
    }
}
