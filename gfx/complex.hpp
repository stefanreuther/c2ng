/**
  *  \file gfx/complex.hpp
  */
#ifndef C2NG_GFX_COMPLEX_HPP
#define C2NG_GFX_COMPLEX_HPP

#include "gfx/point.hpp"
#include "gfx/types.hpp"
#include "afl/base/types.hpp"

namespace gfx {

    class Context;
    class Rectangle;
    class Canvas;

    void drawHLine(const Context& ctx, int x1, int y1, int x2);
    void drawVLine(const Context& ctx, int x1, int y1, int y2);
    void drawLine(const Context& ctx, const Point p1, const Point p2);
    void drawLineTo(Context& ctx, const Point pt);
    void drawLineRel(Context& ctx, int dx, int dy);
    void drawCircle(const Context& ctx, const Point pt, int r);
    void drawFilledCircle(const Context& ctx, const Point pt, int r);
    void drawBar(const Context& ctx, int x1, int y1, int x2, int y2);
    void drawBar(const Context& ctx, const Rectangle& r);
    void drawSolidBar(const Context& ctx, const Rectangle& r, Color_t color);
    // void drawSolidBar(Canvas& can, const Rectangle& r, Color_t color); FIXME: needed?
    void drawRectangle(const Context& ctx, const Rectangle& r);
    void drawArrow(const Context& ctx, const Point p1, const Point p2, int ptsize);
    void drawPixel(const Context& ctx, const Point pt);
    void drawBackground(Context& ctx, const Rectangle& r);

    // Blit operations
    // Parameter order is
    // - context
    // - where it goes (point or rectangle)
    // - the pixmap
    // - options, if any
    void blitPixmap(Context& ctx, Point pt, Canvas& pixmap);
    void blitPixmap(Context& ctx, Point pt, Canvas& pixmap, Rectangle area);
    void blitSized(Context& ctx, Rectangle area, Canvas& pixmap);
    void blitTiled(Context& ctx, const Rectangle& area, Canvas& pixmap, int alt);
    void blitTiledAnchored(Context& ctx, const Rectangle& area, Canvas& pixmap, Point anchor, int alt);

}

#endif
