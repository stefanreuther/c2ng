/**
  *  \file gfx/complex.hpp
  */
#ifndef C2NG_GFX_COMPLEX_HPP
#define C2NG_GFX_COMPLEX_HPP

#include "gfx/point.hpp"
#include "gfx/types.hpp"
#include "afl/base/types.hpp"
#include "gfx/fillpattern.hpp"

namespace gfx {

    class BaseContext;
    class Rectangle;
    class Canvas;

    template<typename Index>
    class Context;

    void drawHLine(const BaseContext& ctx, int x1, int y1, int x2);
    void drawVLine(const BaseContext& ctx, int x1, int y1, int y2);
    void drawLine(const BaseContext& ctx, const Point p1, const Point p2);
    void drawLineTo(BaseContext& ctx, const Point pt);
    void drawLineRel(BaseContext& ctx, int dx, int dy);
    void drawCircle(const BaseContext& ctx, const Point pt, int r);
    void drawFilledCircle(const BaseContext& ctx, const Point pt, int r);
    void drawBar(const BaseContext& ctx, int x1, int y1, int x2, int y2);
    void drawBar(const BaseContext& ctx, const Rectangle& r);
    // void drawSolidBar(Canvas& can, const Rectangle& r, Color_t color); FIXME: needed?
    void drawRectangle(const BaseContext& ctx, const Rectangle& r);
    void drawArrow(const BaseContext& ctx, const Point p1, const Point p2, int ptsize);
    void drawPixel(const BaseContext& ctx, const Point pt);

    template<typename Index>
    void drawSolidBar(const Context<Index>& ctx, const Rectangle& r, typename Context<Index>::Index_t color);

    template<typename Index>
    void drawBackground(Context<Index>& ctx, const Rectangle& r);

    // Blit operations
    // Parameter order is
    // - context
    // - where it goes (point or rectangle)
    // - the pixmap
    // - options, if any
    void blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap);
    void blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap, Rectangle area);
    void blitSized(const BaseContext& ctx, Rectangle area, Canvas& pixmap);
    void blitTiled(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, int alt);
    void blitTiledAnchored(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, Point anchor, int alt);

}

// /** Draw Solid Bar. Draws a solid rectangle with the given color. This
//     one does not use a graphics parameter set.
//     \param can canvas
//     \param r rectangle to fill
//     \param color color to fill the rectangle with */
template<typename Index>
void
gfx::drawSolidBar(const Context<Index>& ctx, const Rectangle& r, typename Context<Index>::Index_t color)
{
    ctx.canvas().drawBar(r,
                         ctx.colorScheme().getColor(color),
                         TRANSPARENT_COLOR,
                         FillPattern::SOLID,
                         ctx.getAlpha());
}

template<typename Index>
void
gfx::drawBackground(Context<Index>& ctx, const Rectangle& r)
{
    ctx.colorScheme().drawBackground(ctx.canvas(), r);
}

#endif
