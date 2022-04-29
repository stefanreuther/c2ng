/**
  *  \file ui/icons/balloon.cpp
  *  \brief Class ui::icons::Balloon
  */

#include "ui/icons/balloon.hpp"
#include "gfx/complex.hpp"

namespace {
    /* Parameters (convert to runtime if needed) */
    const int BORDER = 1;        // Width of border: 1 pixel
    const int PAD = 1;           // Padding
    const int TAIL = 6;          // Size of the tail

    struct Coords {
        int xl, xr, xm, xd, xu, yt, yp, yb;
    };

    void drawFrame(gfx::BaseContext& ctx, const Coords& c)
    {
        drawLine(ctx, gfx::Point(c.xl, c.yt), gfx::Point(c.xl, c.yb));  // left
        drawLine(ctx, gfx::Point(c.xr, c.yt), gfx::Point(c.xr, c.yb));  // right
        drawLine(ctx, gfx::Point(c.xd, c.yb), gfx::Point(c.xm, c.yp));  // down
        drawLine(ctx, gfx::Point(c.xu, c.yb), gfx::Point(c.xm, c.yp));  // up

        drawLine(ctx, gfx::Point(c.xl+1, c.yt), gfx::Point(c.xr-1, c.yt)); // top
        drawLine(ctx, gfx::Point(c.xl+1, c.yb), gfx::Point(c.xd-1, c.yb)); // bottom-left
        drawLine(ctx, gfx::Point(c.xu+1, c.yb), gfx::Point(c.xr-1, c.yb)); // bottom-right
    }
}

ui::icons::Balloon::Balloon(Icon& content, Root& root, uint8_t color)
    : m_content(content), m_root(root), m_color(color)
{ }


bool
ui::icons::Balloon::setColor(uint8_t color)
{
    bool result = (m_color != color);
    m_color = color;
    return result;
}

gfx::Point
ui::icons::Balloon::getSize() const
{
    return m_content.getSize() + gfx::Point(2*PAD + 2*BORDER,
                                            2*PAD + 2*BORDER + TAIL);
}

void
ui::icons::Balloon::draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    //  +------------+   - yt
    //  |            |
    //  +----    ----+   - yb
    //       \  /
    //        \/         - yp
    //  xl xd xm xu xr
    Coords c;
    c.xl = area.getLeftX();
    c.xr = area.getRightX()-1;
    c.xm = area.getLeftX() + area.getWidth()/2;
    c.xd = c.xm - TAIL;
    c.xu = c.xm + TAIL;

    c.yt = area.getTopY();
    c.yp = area.getBottomY()-1;
    c.yb = c.yp - TAIL;

    // Frame
    gfx::Context<uint8_t> ctx8(ctx.canvas(), m_root.colorScheme());
    ctx8.setColor(m_color);
    drawFrame(ctx8, c);

    // Fill with 50% gray (total black in palettized mode)
    if (ctx.canvas().getBitsPerPixel() >= 16) {
        ctx8.setAlpha(128);
    }
    ctx8.setColor(Color_Black);
    drawSolidBar(ctx8, gfx::Rectangle(c.xl+1, c.yt+1, c.xr-c.xl-1, c.yb-c.yt-1), Color_Black);
    for (int i = 0; i < TAIL; ++i) {
        drawHLine(ctx8, c.xd+i+1, c.yb+i, c.xu-i-1);
    }

    // Determine content area
    area.consumeX(BORDER + PAD);
    area.consumeRightX(BORDER + PAD);
    area.consumeY(BORDER + PAD);
    area.consumeBottomY(BORDER + PAD + TAIL);

    // Draw content
    m_content.draw(ctx, area, flags);
}
