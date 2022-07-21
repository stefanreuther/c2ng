/**
  *  \file gfx/antialiased.hpp
  *  \brief Antialiased Graphics Primitives
  */
#ifndef C2NG_GFX_ANTIALIASED_HPP
#define C2NG_GFX_ANTIALIASED_HPP

#include "gfx/point.hpp"

namespace gfx {

    class BaseContext;

    /** Draw line, with anti-aliasing.
        Draws a line from p1 to p2, inclusive.

        For now, supports only solid, 1-pixel, opaque lines.
        If different parameters are used, a normal line is drawn instead.

        @param ctx Context (color, canvas, line pattern, thickness, alpha)
        @param p1  First point
        @param p2  Second point */
    void drawLineAA(BaseContext& ctx, Point p1, Point p2);

    /** Draw circle, with anti-aliasing.
        Draws a circle of radius r around pt.
        We draw pixels closer than r+1 from center.

        For now, supports only solid, 1-pixel, opaque lines.
        If different parameters are used, a normal circle is drawn instead.

        @param ctx Context (color, canvas, line pattern, thickness, alpha)
        @param pt  Center point
        @param r   Radius */
    void drawCircleAA(BaseContext& ctx, Point pt, int r);

}

#endif
