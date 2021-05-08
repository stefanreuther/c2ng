/**
  *  \file gfx/blit.cpp
  *  \brief Stretch / Shear / Rotate Blit
  *
  *  This module contains operations for transformed blitting. We use
  *  a general approach to blit a rectangular pixmap to an arbitrary
  *  parallelogram.
  *
  *  \todo there's much that could be improved here: use a faster
  *  algorithm (some sort of Bresenham), anti-aliasing, etc.
  */

#include <algorithm>
#include <cmath>
#include "gfx/blit.hpp"

void
gfx::blitStretchRotate(Canvas& src, Canvas& target, Rectangle src_clip, Rectangle target_clip, int x, int y, int x1, int y1, int x2, int y2)
{
    // Check input area
    src_clip.intersect(Rectangle(Point(), src.getSize()));
    if (!src_clip.exists()) {
        /* input is empty */
        return;
    }

    // Check output area
    target_clip.intersect(Rectangle(Point(), target.getSize()));
    if (!target_clip.exists()) {
        /* input is empty */
        return;
    }

    // Determinant
    const int det = (x2*y1 - y2*x1);
    if (det == 0) {
        /* output axes are parallel or anti-parallel */
        return;
    }

    // Do it
    for (int yp = 0, y_out = target_clip.getTopY(); yp < target_clip.getHeight(); ++yp, ++y_out) {
        for (int xp = 0, x_out = target_clip.getLeftX(); xp < target_clip.getWidth(); ++xp, ++x_out) {
            const int xpp = x_out - x;
            const int ypp = y_out - y;
            const int x0 = src_clip.getWidth() * (x2 * ypp - y2 * xpp) / det;
            const int y0 = src_clip.getHeight() * (y1 * xpp - x1 * ypp) / det;
            if (x0 < 0 || y0 < 0 || x0 >= src_clip.getWidth() || y0 >= src_clip.getHeight()) {
                ;
            } else {
                // Copy pixel
                // FIXME: optimize to perform block copy
                Color_t colorBuffer[1];
                ColorQuad_t quadBuffer[1];
                src.getPixels(Point(x0 + src_clip.getLeftX(), y0 + src_clip.getTopY()), colorBuffer);
                src.decodeColors(colorBuffer, quadBuffer);
                target.encodeColors(quadBuffer, colorBuffer);
                target.drawPixels(Point(x_out, y_out), colorBuffer, OPAQUE_ALPHA);
            }
        }
    }
}

gfx::Rectangle
gfx::computeStretchRotateBBox(int x1, int y1, int x2, int y2)
{
    return Rectangle(std::min(x1, 0) + std::min(x2, 0),
                     std::min(y1, 0) + std::min(y2, 0),
                     std::abs(x1) + std::abs(x2),
                     std::abs(y1) + std::abs(y2));
}
