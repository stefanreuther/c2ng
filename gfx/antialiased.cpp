/**
  *  \file gfx/antialiased.cpp
  *  \brief Antialiased Graphics Primitives
  */

#include <algorithm>
#include "gfx/antialiased.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/complex.hpp"

// Draw line, with anti-aliasing.
void
gfx::drawLineAA(BaseContext& ctx, Point p1, Point p2)
{
    /* For simplicity, this maps all lines into four octants,
       and decides between two cases handling two of them each.
       To allow for lines that nicely connect to each other, we'd need to
       exclude x2,y2 from drawing, and implement eight octants (as four
       cases). The initialisation of the error term might also need
       adjustment. */
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

    /* Reject unsupported cases */
    if (ctx.getLineThickness() != 1 || ctx.getLinePattern() != 0xFF || ctx.getAlpha() != OPAQUE_ALPHA) {
        drawLine(ctx, p1, p2);
        return;
    }

    Canvas& can = ctx.canvas();
    const Color_t color = ctx.getRawColor();

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

        int error = dy >> 1;
        int addx  = (x2 < x1) ? -1 : 1;

        /* The error term is the distance between the actual exact
           line and the pixel we set. It directly maps to an alpha
           value. Since our alpha runs from 0..255, the mapping is
           alpha=255*error/dy. To avoid dividing in a loop, we
           represent alpha as its integer part, and the fractional
           part as numerator of a fraction with base dy, and increment
           that step by step. */

        // Initial 'alpha = 255 * error / dy'. Can probably be simplified.
        uint8_t alpha       = static_cast<uint8_t>(255 * error / dy);
        int     alpha_fract =                      255 * error % dy;

        // alpha increments by 255*dx/dy each iteration.
        uint8_t alpha_inc       = static_cast<uint8_t>(255 * dx / dy);
        int     alpha_fract_inc =                      255 * dx % dy;

        while (y1 <= y2) {
            // draw pixel
            can.drawPixel(Point(x1,      y1), color, static_cast<uint8_t>(~alpha));
            can.drawPixel(Point(x1+addx, y1), color,                       alpha);

            // increment y, and error term (fractional X position)
            y1++;
            error += dx;

            // increment alpha
            alpha = static_cast<uint8_t>(alpha + alpha_inc);
            alpha_fract += alpha_fract_inc;
            if (alpha_fract >= dy) {
                alpha_fract -= dy;
                ++alpha;
            }

            if(error >= dy) {
                /* decrementing error by dy means decrementing alpha
                   by 255*dy/dy, i.e. incrementing by one */
                error -= dy;
                ++alpha;
                x1 += addx;
            }
        }
    } else {                // X is the major axis
        if (x1 > x2) {
            std::swap(y1, y2);
            std::swap(x1, x2);
        }

        int error = dx >> 1;
        int addy  = (y2 < y1) ? -1 : 1;

        uint8_t alpha       = static_cast<uint8_t>(255 * error / dx);
        int     alpha_fract =                      255 * error % dx;

        uint8_t alpha_inc       = static_cast<uint8_t>(255 * dy / dx);
        int     alpha_fract_inc =                      255 * dy % dx;

        while(x1 <= x2) {
            can.drawPixel(Point(x1, y1),      color, static_cast<uint8_t>(~alpha));
            can.drawPixel(Point(x1, y1+addy), color,                       alpha);
            x1++;
            error += dy;
            alpha = static_cast<uint8_t>(alpha + alpha_inc);
            alpha_fract += alpha_fract_inc;
            if (alpha_fract >= dx) {
                alpha_fract -= dx;
                ++alpha;
            }
            if (error >= dx) {
                error -= dx;
                ++alpha;
                y1 += addy;
            }
        }
    }
}

// Draw circle, with anti-aliasing.
void
gfx::drawCircleAA(BaseContext& ctx, Point pt, int r)
{
    /* Reject unsupported case */
    if (ctx.getLineThickness() != 1 || ctx.getLinePattern() != 0xFF || ctx.getAlpha() != OPAQUE_ALPHA) {
        drawCircle(ctx, pt, r);
        return;
    }

    const int x0 = pt.getX();
    const int y0 = pt.getY();
    int w = 0;
    int k = 1;
    int z = 0;
    int y2 = 0;
    int xk = 2*r-1;
    int x = r;

    /* Given x, we need to compute y = sqrt(r**2 - x**2), or
       y**2 = r**2 - x**2. Starting with x=r, y starts at 0.

       Define z = w**2, and z >= y**2.

       We can incrementally compute exact values for y**2 = y2 using
       the identity that y**2 = \sum_{i=0..y} 2y-1 (bottom of outer loop).

       To find the smallest z such that z >= y**2, we compute the z and w
       values using the same identity (inner loop).

       This generates an y value for all x \in [0,r]. Since we're only
       interested in one octant, we filter out cases where w<x. */
    Canvas& can = ctx.canvas();
    const Color_t color = ctx.getRawColor();
    while (x >= 0) {
        while (z < y2) {
            z += k;
            ++w;
            k += 2;
        }
        if (w >= x) {
            /* The actual circle y2 lies between z and next z, i.e.
               z and z+k. Look where we are. This is not mathematically
               clean (should have some roots and squares here), but
               looks well in practice. */
            uint8_t alpha = static_cast<uint8_t>((z - y2) * 255 / k);
            uint8_t nalpha = static_cast<uint8_t>(~alpha);

            can.drawPixel(Point(x0 - x, y0 - w), color, nalpha);
            can.drawPixel(Point(x0 - x, y0 - w+1), color, alpha);

            can.drawPixel(Point(x0 + x, y0 - w), color, nalpha);
            can.drawPixel(Point(x0 + x, y0 - w+1), color, alpha);

            can.drawPixel(Point(x0 - w, y0 - x), color, nalpha);
            can.drawPixel(Point(x0 - w+1, y0 - x), color, alpha);

            can.drawPixel(Point(x0 + w, y0 - x), color, nalpha);
            can.drawPixel(Point(x0 + w-1, y0 - x), color, alpha);

            can.drawPixel(Point(x0 - w, y0 + x), color, nalpha);
            can.drawPixel(Point(x0 - w+1, y0 + x), color, alpha);

            can.drawPixel(Point(x0 + w, y0 + x), color, nalpha);
            can.drawPixel(Point(x0 + w-1, y0 + x), color, alpha);

            can.drawPixel(Point(x0 - x, y0 + w), color, nalpha);
            can.drawPixel(Point(x0 - x, y0 + w-1), color, alpha);

            can.drawPixel(Point(x0 + x, y0 + w), color, nalpha);
            can.drawPixel(Point(x0 + x, y0 + w-1), color, alpha);
        }
        y2 += xk;
        xk -= 2;
        --x;
    }
}
