/**
  *  \file gfx/complex.hpp
  *  \brief Complex Graphics Primitives
  *
  *  This module contains graphics primitives built on the Canvas operations.
  *  They typically use a BaseContext to retrieve parameters.
  */
#ifndef C2NG_GFX_COMPLEX_HPP
#define C2NG_GFX_COMPLEX_HPP

#include "afl/base/types.hpp"
#include "gfx/fillpattern.hpp"
#include "gfx/point.hpp"
#include "gfx/types.hpp"

namespace gfx {

    class BaseContext;
    class Rectangle;
    class Canvas;

    template<typename Index>
    class Context;

    /** Draw Horizontal Line.
        Draw a horizontal line from (x1,y1) to (x2,y1), inclusive.
        There are no restrictions regarding the relations between the ending points.
        \param ctx Context (color, thickness, line pattern, alpha)
        \param x1,y1 starting coordinates
        \param x2 ending X. */
    void drawHLine(const BaseContext& ctx, int x1, int y1, int x2);

    /** Draw Vertical Line.
        Draw a vertical line from (x1,y1) to (x1,y2), inclusive.
        There are no restrictions regarding the relations between the ending points.
        \param ctx Context (color, thickness, line pattern, alpha)
        \param x1,y1 starting coordinates
        \param y2 ending Y */
    void drawVLine(const BaseContext& ctx, int x1, int y1, int y2);

    /** Draw Line.
        Draws a general line from (x1,y1) to (x2,y2), inclusive.
        This is a standard Bresenham algorithm, with shortcuts for horizontal and vertical lines.
        There are no restrictions regarding the relations between the endpoints.
        \param ctx Context (color, thickness, line pattern, alpha)
        \param p1 starting coordinates
        \param p2 ending coordinates */
    void drawLine(const BaseContext& ctx, const Point p1, const Point p2);

    /** Draw Line to.
        Draws a line between the context's current graphics cursor to the specified point.
        Otherwise like drawLine(), i.e. endpoints are inclusive.
        The graphics cursor is updated.
        \param ctx Context (color, thickness, line pattern, alpha, cursor)
        \param pt ending coordinates*/
    void drawLineTo(BaseContext& ctx, const Point pt);

    /** Draw Relative Line.
        Draws a line from the context's current graphics cursor to the point offset by (dx,dy) from it.
        Otherwise like drawLine().
        \param ctx Context (color, thickness, line pattern, alpha, cursor)
        \param dx,dy offset of endpoint relative to cursor. */
    void drawLineRel(BaseContext& ctx, int dx, int dy);

    /** Circle.
        Draws a circle around pt, with the specified radius.

        The algorithm variation used here draws pixels that are less than r+1 from center.

        \todo some pixels in the diagonals are painted twice which is a
        Bad Thing(tm) for alpha circles

        \param ctx Context (color, thickness, line pattern, alpha)
        \param pt center point
        \param r radius */
    void drawCircle(const BaseContext& ctx, const Point pt, int r);

    /** Draw filled circle.
        The circle is filled with the current fill settings.

        \param ctx Context (color, background color, fill pattern, alpha)
        \param pt center point
        \param r radius */
    void drawFilledCircle(const BaseContext& ctx, const Point pt, int r);

    /** Draw filled bar (filled rectangle).
        Draws a filled bar between the corners (x1,y1) and (x2,y2), inclusive.
        The given points can be any two opposing corners of the rectangle.

        \param ctx Context (color, background color, fill pattern, alpha)
        \param x1,y1 one corner
        \param x2,y2 other corner */
    void drawBar(const BaseContext& ctx, int x1, int y1, int x2, int y2);

    /** Draw filled bar (filled rectangle).
        Draws a filled bar at the coordinates given by a Rectangle.
        \param ctx Context (color, background color, fill pattern, alpha)
        \param r rectangle to fill */
    void drawBar(const BaseContext& ctx, const Rectangle& r);

    /** Draw rectangle.
        Draw the borders of a rectangle, but does not fill the object.
        Degenerate rectangles (size 0 or 1) are supported.
        \param ctx Context (color, thickness, line pattern, alpha)
        \param r rectangle to draw */
    void drawRectangle(const BaseContext& ctx, const Rectangle& r);

    /** Draw an arrow.
        \param ctx    Context (color, thickness, line pattern, alpha)
        \param p1     Starting coordinates
        \param p2     Ending coordinates
        \param ptsize Size of pointer (arrowhead) added at p2 */
    void drawArrow(const BaseContext& ctx, const Point p1, const Point p2, int ptsize);

    /** Draw pixel.
        \param ctx  Context (color, alpha)
        \param pt   Point */
    void drawPixel(const BaseContext& ctx, const Point pt);

    /** Draw filled bar with color.
        Draws a solid rectangle with the given color.
        \param ctx   Context (color scheme, alpha)
        \param r     Rectangle to fill
        \param color Fill color to use */
    template<typename Index>
    void drawSolidBar(const Context<Index>& ctx, const Rectangle& r, typename Context<Index>::Index_t color);

    /** Draw background.
        This is a convenience method for ColorScheme::drawBackground().
        \param ctx   Context (color scheme)
        \param r     Rectangle to fill */
    template<typename Index>
    void drawBackground(Context<Index>& ctx, const Rectangle& r);

    /** Draw a filled polygon.

        \param ctx Context (color, fill pattern, alpha)
        \param pts Points (at least 3) */
    void drawFilledPolygon(const BaseContext& ctx, afl::base::Memory<const Point> pts);


    /*
     *  Blit operations
     *
     *  Parameter order is
     *  - context
     *  - where it goes (point or rectangle)
     *  - the pixmap
     *  - options, if any
     */

    /** Blit pixmap on canvas.
        Displays this pixmap on the specified canvas, at the specified position.
        \param ctx    Context
        \param pt     Point (pixmap's top-left corner goes here)
        \param pixmap Pixmap to blit */
    void blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap);

    /** Blit pixmap on canvas.
        Displays a sub-area of this pixmap on the specified canvas.

        \param ctx    Context
        \param pt     Point (top-left pixel of area goes here)
        \param pixmap Pixmap
        \param area   Area of this pixmap to use, in pixmap coordinates.

        \note The point specifies the coordinates of the upper-left point *after* clipping the pixmap at area.
        For Canvas::blit(), they specify the coordinates *before* clipping. */
    void blitPixmap(const BaseContext& ctx, Point pt, Canvas& pixmap, Rectangle area);

    /** Blit pixmap on canvas.
        Ensures that exactly the required area is covered:
        if the pixmap is too large, it is clipped;
        if the pixmap is too small, the outside is filled with the context's current color.

        \param ctx    Context
        \param area   Area to cover
        \param pixmap Pixmap to blit */
    void blitSized(const BaseContext& ctx, Rectangle area, Canvas& pixmap);

    /** Tile area with pixmap.
        The pixmap is repeated as needed to cover the entire area.
        If alt is nonzero, every odd row will be offset by this amount of pixels,
        to make the tiling a little more interesting.

        \param ctx    Context
        \param area   Area to tile
        \param pixmap Pixmap to blit
        \param alt    Alteration of X coordinate
        \see tileAnchored() */
    void blitTiled(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, int alt);

    /** Tile area with pixmap, anchored.
        Same as blitTiled(), but with a fixed anchor point.

        \param ctx    Context
        \param area   Area to tile
        \param pixmap Pixmap to blit
        \param anchor Anchor point
        \param alt    Alteration of X coordinate

        To have multiple tiled regions match to each other nicely, use the same anchor point.
        Using blitTiled() instead will anchor the tiles at the top-left point of the area;
        this way, the join points will be visible. */
    void blitTiledAnchored(const BaseContext& ctx, const Rectangle& area, Canvas& pixmap, Point anchor, int alt);

}

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
