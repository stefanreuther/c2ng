/**
  *  \file gfx/complex.cpp
  *  \brief Complex Graphics Primitives
  */

#include <algorithm>
#include <cmath>
#include "gfx/complex.hpp"
#include "gfx/canvas.hpp"
#include "gfx/colorscheme.hpp"
#include "gfx/context.hpp"
#include "gfx/fillpattern.hpp"
#include "util/math.hpp"

using afl::base::Memory;

namespace {
#define LINE_PATTERN1(parm, t) ((parm).getLinePattern() & (0x80 >> (t & 7)))

    /// Helper function for 1-wide lines.
    void linePixel(const gfx::BaseContext& c, int x1, int y1)
    {
        gfx::drawPixel(c, gfx::Point(x1, y1));
    }

    /// Helper function for general lines in north-south direction.
    void lineHLine(const gfx::BaseContext& c, int x1, int y1)
    {
        c.canvas().drawHLine(gfx::Point(x1, y1), c.getLineThickness(), c.getRawColor(), gfx::SOLID_LINE, c.getAlpha());
    }

    /// Helper function for general lines in east-west direction.
    void lineVLine(const gfx::BaseContext& c, int x1, int y1)
    {
        c.canvas().drawVLine(gfx::Point(x1, y1), c.getLineThickness(), c.getRawColor(), gfx::SOLID_LINE, c.getAlpha());
    }

    /// Compute add+a/b with rounding.
    inline int32_t divideAndRound(int32_t add, int32_t a, int32_t b)
    {
        a += add*b;             // for correct rounding if a/b is negative but add+a/b is positive
        a += b/2;               // rounding...
        return a / b;
    }

}

void
gfx::drawHLine(const BaseContext& ctx, int x1, int y1, int x2)
{
    // ex gfx/gfxcompl.h:drawHLine
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    x2++;

    if (ctx.getLineThickness() == 1) {
        ctx.canvas().drawHLine(Point(x1, y1), x2-x1, ctx.getRawColor(), ctx.getLinePattern(), ctx.getAlpha());
    } else {
        ctx.canvas().drawBar(Rectangle(x1, y1 - ctx.getLineThickness()/2, x2-x1, ctx.getLineThickness()),
                             ctx.getRawColor(),
                             TRANSPARENT_COLOR,
                             FillPattern(ctx.getLinePattern()),
                             ctx.getAlpha());
    }
}

void
gfx::drawVLine(const BaseContext& ctx, int x1, int y1, int y2)
{
    // ex gfx/gfxcompl.h:drawVLine
    if (y2 < y1) {
        std::swap(y1, y2);
    }
    y2++;

    if (ctx.getLineThickness() == 1) {
        ctx.canvas().drawVLine(Point(x1, y1), y2-y1, ctx.getRawColor(), ctx.getLinePattern(), ctx.getAlpha());
    } else {
        FillPattern pat;
        for (int i = 0; i < 8; ++i) {
            if ((ctx.getLinePattern() << i) & 0x80) {
                pat[i] = 0xFF;
            }
        }
        ctx.canvas().drawBar(Rectangle(x1 - ctx.getLineThickness()/2, y1, ctx.getLineThickness(), y2-y1),
                             ctx.getRawColor(),
                             TRANSPARENT_COLOR,
                             pat,
                             ctx.getAlpha());
    }
}

void
gfx::drawLine(const BaseContext& ctx, const Point p1, const Point p2)
{
    int x1 = p1.getX(), y1 = p1.getY();
    int x2 = p2.getX(), y2 = p2.getY();

    /* Handle easy cases first */
    if (x1 == x2) {
        drawVLine(ctx, x1, y1, y2);
        return;
    }
    if (y1 == y2) {
        drawHLine(ctx, x1, y1, x2);
        return;
    }

    /* Vanilla Bresenham */
    int dx = x2 - x1;
    int dy = y2 - y1;
    if(dx < 0) {
        dx = -dx;
    }
    if(dy < 0) {
        dy = -dy;
    }

    if(dx < dy) {                // Y is the major axis
        if(y1 > y2) {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

        void (*plot) (const BaseContext&,int,int);
        int error = dy >> 1;
        int addx  = (x2 < x1) ? -1 : 1;

        if(ctx.getLineThickness() == 1) {
            plot = linePixel;
        } else {
            /* FIXME: correct error term for even line thicknesses */
            x1 -= ctx.getLineThickness() / 2;
            plot = lineHLine;
        }

        while(y1 <= y2) {
            if (LINE_PATTERN1(ctx, y1)) {
                plot(ctx, x1, y1);
            }
            y1++;
            error += dx;
            if(error >= dy) {
                error -= dy;
                x1 += addx;
            }
        }
    } else {                // X is the major axis
        if(x1 > x2) {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

        void (*plot) (const BaseContext&,int,int);
        int error = dx >> 1;
        int addy  = (y2<y1) ? -1 : 1;

        if(ctx.getLineThickness() == 1) {
            plot = linePixel;
        } else {
            /* FIXME: correct error term for even line thicknesses */
            y1 -= ctx.getLineThickness() / 2;
            plot = lineVLine;
        }

        while(x1 <= x2) {
            if (LINE_PATTERN1(ctx, x1)) {
                plot(ctx, x1, y1);
            }
            x1++;
            error += dy;
            if(error >= dx) {
                error -= dx;
                y1 += addy;
            }
        }
    }
}

void
gfx::drawLineTo(BaseContext& ctx, const Point pt)
{
    // ex gfx/gfxcompl.h:drawLineTo
    drawLine(ctx, ctx.getCursor(), pt);
    ctx.setCursor(pt);
}

void
gfx::drawLineRel(BaseContext& ctx, int dx, int dy)
{
    // ex gfx/gfxcompl.h:drawLineRel
    drawLineTo(ctx, ctx.getCursor() + Point(dx, dy));
}

void
gfx::drawCircle(const BaseContext& ctx, const Point pt, int r)
{
    /*
     *  A home-brew circle algorithm (but probably not at all new):
     *  . it uses the equation sum(1,3,5,...,2n-1) = n^2 to compute
     *    pythagoras' theorem.
     *  . I first implemented it in Z80 assembly language
     *  If you have a better algorithm, tell me.
     */
    const int x0 = pt.getX();
    const int y0 = pt.getY();
#if 1
    /* FIXME: This is the circle routine used in PCC I, which does not
       support line-patterned circles and is quite slow. The other one
       below has different (=wrong) rounding characteristics. We want
       a circle of radius 3 to appear as a "rounded rectangle" with
       diameter 7, the other draws squares with diameter 5 instead. */
    register int w = 0;
    register int k = 1;
    register int z = 0;
    register int y2 = 0;
    register int xk = 2*r-1;
    register int x = r;
    Canvas& can = ctx.canvas();
    const Color_t color = ctx.getRawColor();
    const Memory<const Color_t> colorMem = Memory<const Color_t>::fromSingleObject(color);
    const Alpha_t alpha = ctx.getAlpha();
    while (x >= 0) {
        while (z < y2) {
            z += k;
            ++w;
            k += 2;
        }
        if (w >= x) {
            can.drawPixels(Point(x0 - x, y0 - w), colorMem, alpha);
            can.drawPixels(Point(x0 + x, y0 - w), colorMem, alpha);
            can.drawPixels(Point(x0 - w, y0 - x), colorMem, alpha);
            can.drawPixels(Point(x0 + w, y0 - x), colorMem, alpha);
            can.drawPixels(Point(x0 - w, y0 + x), colorMem, alpha);
            can.drawPixels(Point(x0 + w, y0 + x), colorMem, alpha);
            can.drawPixels(Point(x0 - x, y0 + w), colorMem, alpha);
            can.drawPixels(Point(x0 + x, y0 + w), colorMem, alpha);
        }
        y2 += xk;
        xk -= 2;
        --x;
    }
#else
    register int w = 0;
    register int k = 1;
    register int z = 0;
    register int y2 = 0;
    register int xk = 2*r-1;
    register int x = r;

    register int w0 = w;
    while(x >= w) {
        while(z < y2) {
            z += k;
            drawVLine(ctx, x0+x, y0+w0, y0+w);
            drawVLine(ctx, x0-x, y0+w0, y0+w);
            drawHLine(ctx, x0+w0, y0+x, x0+w);
            drawHLine(ctx, x0+w0, y0-x, x0+w);
            if (w) {
                drawVLine(ctx, x0+x, y0-w0, y0-w);
                drawVLine(ctx, x0-x, y0-w0, y0-w);
                drawHLine(ctx, x0-w0, y0+x, x0-w);
                drawHLine(ctx, x0-w0, y0-x, x0-w);
            }
            w++;
            w0 = w;
            k += 2;
        }
        y2 += xk;
        xk -= 2;
        x--;
    }
#endif
}

void
gfx::drawFilledCircle(const BaseContext& ctx, const Point pt, int r)
{
    /* Roughly the same algorithm as drawCircle */
    const int x0 = pt.getX();
    const int y0 = pt.getY();
    register int w = 0;
    register int k = 1;
    register int z = 0;
    register int y2 = 0;
    register int xk = 2*r-1;
    register int x = r;

    register int w0 = w;
    while(x >= 0) {
        while(z < y2) {
            z += k;
            drawBar(ctx, x0 - x, y0 + w0, x0 + x, y0 + w);
            if (w)
                drawBar(ctx, x0 - x, y0 - w0, x0 + x, y0 - w);
            w++;
            w0 = w;
            k += 2;
        }
        y2 += xk;
        xk -= 2;
        x--;
    }
}

void
gfx::drawBar(const BaseContext& ctx, int x1, int y1, int x2, int y2)
{
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
    }

    ctx.canvas().drawBar(Rectangle(x1, y1, x2-x1+1, y2-y1+1),
                         ctx.getRawColor(),
                         TRANSPARENT_COLOR,
                         ctx.fillPattern(),
                         ctx.getAlpha());
}

void
gfx::drawBar(const BaseContext& ctx, const Rectangle& r)
{
    ctx.canvas().drawBar(r,
                         ctx.getRawColor(),
                         TRANSPARENT_COLOR,
                         ctx.fillPattern(),
                         ctx.getAlpha());
}

void
gfx::drawRectangle(const BaseContext& ctx, const Rectangle& r)
{
    if (r.getWidth() == 0 || r.getHeight() == 0) {
        return;
    } else if (r.getWidth() == 1) {
        drawVLine(ctx, r.getLeftX(), r.getTopY(), r.getBottomY() - 1);
    } else if (r.getHeight() == 1) {
        drawHLine(ctx, r.getLeftX(), r.getTopY(), r.getRightX() - 1);
    } else {
        int x2 = r.getRightX() - 1;
        int y2 = r.getBottomY() - 1;
        drawHLine(ctx, r.getLeftX(), r.getTopY(), x2);
        drawHLine(ctx, r.getLeftX(),  y2, x2);
        drawVLine(ctx, r.getLeftX(), r.getTopY()+1, y2-1);
        drawVLine(ctx, x2,  r.getTopY()+1, y2-1);
    }
}

void
gfx::drawArrow(const BaseContext& ctx, const Point p1, const Point p2, int ptsize)
{
    drawLine(ctx, p1, p2);

    int len = int(std::sqrt(double(util::squareInteger(p2.getX() - p1.getX()) + util::squareInteger(p2.getY() - p1.getY()))));
    if (len) {
        int dx = (p2.getX() - p1.getX()) * ptsize / len;
        int dy = (p2.getY() - p1.getY()) * ptsize / len;
        drawLine(ctx, p2, p2 + Point(-dy-dx, +dx-dy));
        drawLine(ctx, p2, p2 + Point(+dy-dx, -dx-dy));
    }
}

void
gfx::drawPixel(const BaseContext& ctx, const Point pt)
{
    const Color_t color = ctx.getRawColor();
    const Memory<const Color_t> colorMem = Memory<const Color_t>::fromSingleObject(color);
    ctx.canvas().drawPixels(pt, colorMem, ctx.getAlpha());
}

void
gfx::drawFilledPolygon(const BaseContext& ctx, afl::base::Memory<const Point> pts)
{
    // Determine number of points
    size_t n = pts.size();
    if (n < 3) {
        return;
    }

    // Determine minimum Y
    int y = pts.at(0)->getY();
    for (size_t i = 1; i < n; ++i) {
        y = std::min(y, pts.at(i)->getY());
    }

    // Storage for X's
    std::vector<int> xs;
    xs.reserve(n);

    // Draw starting at y
    while (1) {
        // Determine list of X's intersected by this Y
        xs.clear();
        const Point* p = pts.at(n-1);
        for (size_t i = 0; i < n; ++i) {
            const Point* q = pts.at(i);
            int py = p->getY(), qy = q->getY();
            if ((py <= y && qy > y) || (qy <= y && py > y)) {
                int px = p->getX(), qx = q->getX();
                xs.push_back(divideAndRound(px, (y-py) * (qx-px), (qy-py)));
            }
            p = q;
        }
        if (xs.empty()) {
            break;
        }
        std::sort(xs.begin(), xs.end());

        // Draw horizontal lines
        const LinePattern_t pat = ctx.fillPattern()[y];
        for (size_t i = 0; i+1 < xs.size(); i += 2) {
            ctx.canvas().drawHLine(Point(xs[i], y), xs[i+1] - xs[i], ctx.getRawColor(), pat, ctx.getAlpha());
        }
        ++y;
    }
}

void
gfx::blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap)
{
    // ex GfxPixmap::blit
    ctx.canvas().blit(pt, pixmap, Rectangle(Point(), pixmap.getSize()));
}

void
gfx::blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap, Rectangle area)
{
    // ex GfxPixmap::blit
    pt -= area.getTopLeft();
    area.intersect(Rectangle(Point(), pixmap.getSize()));
    ctx.canvas().blit(pt, pixmap, area);
}

void
gfx::blitSized(const BaseContext& ctx, Rectangle area, Canvas& pixmap)
{
    // ex GfxPixmap::blitSized

    int pix_rect_x = 0;
    int pix_rect_y = 0;
    int pix_rect_w = pixmap.getSize().getX();
    int pix_rect_h = pixmap.getSize().getY();

    int x = area.getLeftX();
    int y = area.getTopY();
    int wi = area.getWidth();
    int he = area.getHeight();

    if (pix_rect_h > he) {
        /* pixmap is bigger than our width -- reduce */
        pix_rect_y += (pix_rect_h - he) / 2;
        pix_rect_h = he;
    } else if (pix_rect_h < he) {
        /* pixmap is too small. Draw outside */
        int overlap = he - pix_rect_h, otop = overlap/2, obot = overlap - otop;
        ctx.canvas().drawBar(Rectangle(x, y,             wi, otop), ctx.getRawColor(), 0, FillPattern::SOLID, OPAQUE_ALPHA);
        ctx.canvas().drawBar(Rectangle(x, y + he - obot, wi, obot), ctx.getRawColor(), 0, FillPattern::SOLID, OPAQUE_ALPHA);
        y += otop;
        he = pix_rect_h;
    }

    if (pix_rect_w > wi) {
        /* pixmap is bigger than our width -- reduce */
        pix_rect_x += (pix_rect_w - wi) / 2;
        pix_rect_w = wi;
    } else if (pix_rect_w < wi) {
        /* pixmap is too small. Draw outside */
        /* |   |XXXX|   |
           x            x+wi */
        int overlap = wi - pix_rect_w, oleft = overlap/2, oright = overlap - oleft;
        ctx.canvas().drawBar(Rectangle(x,               y, oleft,  he), ctx.getRawColor(), 0, FillPattern::SOLID, OPAQUE_ALPHA);
        ctx.canvas().drawBar(Rectangle(x + wi - oright, y, oright, he), ctx.getRawColor(), 0, FillPattern::SOLID, OPAQUE_ALPHA);
        x += oleft;
        wi = pix_rect_w;
    }
    ctx.canvas().blit(Point(x - pix_rect_x, y - pix_rect_y), pixmap, Rectangle(pix_rect_x, pix_rect_y, pix_rect_w, pix_rect_h));
}

void
gfx::blitTiled(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, int alt)
{
    // ex GfxPixmap::tile
    blitTiledAnchored(ctx, area, pixmap, area.getTopLeft(), alt);
}

void
gfx::blitTiledAnchored(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, Point anchor, int alt)
{
    // ex GfxPixmap::tileAnchored
    int y2 = area.getBottomY(), x2 = area.getRightX();
    int y = anchor.getY(), x0 = anchor.getX();
    int mult = -1;
    Point size = pixmap.getSize();

    while (y < y2) {
        int x = x0;
        while (x < x2) {
            Rectangle blit_rect(Point(), pixmap.getSize());

            Rectangle rclip = area;
            rclip.moveBy(Point(-x, -y));
            blit_rect.intersect(rclip);
            if (blit_rect.exists()) {
                ctx.canvas().blit(Point(x, y), pixmap, blit_rect);
            }
            x += size.getX();
        }
        x0 += mult * alt;
        mult = -mult;
        y += size.getY();
    }
}
