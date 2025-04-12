/**
  *  \file client/marker.cpp
  *  \brief Marker drawing
  */

#include <algorithm>
#include "client/marker.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/base/countof.hpp"
#include "gfx/complex.hpp"
#include "gfx/canvas.hpp"

using afl::base::Memory;
using gfx::Color_t;

struct client::Marker {
    int height;
    const int8_t* data;
};


/*
 *  Marker definitions
 */

namespace {
    /* Let's simplify definition of markers a little. We must put these
       into an anonymous namespace to allow forward-declarations. PCC 1.x
       optimizes space usage by overlapping the coordinate arrays where
       possible; we probably needn't do that here as our data segment is
       not limited to 64k :-) */
#define END -99
#define DEFINE_MARKER(name, height)                             \
    extern const int8_t name ## _data[];                        \
    const client::Marker name = { height, name ## _data };      \
    const int8_t name ## _data[]

    /* Normal "user" markers */

    /* "+", normal size; type code 0 */
    DEFINE_MARKER(um_plus, 4) = {
        -3, 0, -2, 0, -1, 0, 1, 0, 2, 0, 3, 0,
        0, -3, 0, -2, 0, -1, 0, 1, 0, 2, 0, 3, END
    };

    /* "!", normal size; type code 1 */
    DEFINE_MARKER(um_exclam, 3) = {
        -1, -3, -1, -2, -1, -1, -1, 0, -1, 2, END
    };

    /* "x", normal size; type code 2 */
    DEFINE_MARKER(um_times, 3) = {
        -2, -2, -1, -1, 0, 0, 1, 1, 2, 2, -2, 2, -1, 1, 1, -1, 2,-2, END
    };

    /* diamond, normal size; type code 3 */
    DEFINE_MARKER(um_diamond, 3) = {
        -2, 0, -1, 1, 2, 0, 1, 1, 0, 2, 1, -1, 0, -2, -1,-1, END
    };

    /* "p" (flag), normal size; type code 4 */
    DEFINE_MARKER(um_flag, 1) = {
        0, 0, 0, -1, 0, -2, 0, -3, 0,-4, 0, -5,
        1, -5, 2, -5, 3, -4, 3, -3, 2, -2, 1, -2, END
    };

    /* up/down arrow, normal size; type code 5 */
    DEFINE_MARKER(um_updown, 4) = {
        -2, 3, -1, 3, 0, 3, 1, 3, 2, 3, -2, -3,
        -1, -3, 0, -3, 1, -3, 2, -3,
        -2, -2, -1, -1, 0, 0, 1, 1, 2, 2, -2, 2,
        -1, 1, 1, -1, 2, -2, END
    };

    /* left/right arrow, normal size; type code 6 */
    DEFINE_MARKER(um_leftright, 3) = {
        -2, -2, -1, -1, 0, 0, 1, 1, 2, 2, -2, 2,
        -1, 1, 1, -1, 2, -2,
        -3, -2, -3, -1, -3, 0, -3, 1, -3, 2, 3, -2,
        3, -1, 3, 0, 3, 1, 3, 2, END
    };

    /* cactus, normal size: type code 7 */
    DEFINE_MARKER(um_cactus, 1) = {
        0, 0, 0, -1, 0, -2, 0, -3, 0, -4, 0, -5,
        0, -6, -1, -3, -2, -4, -2, -5, +1, -2, +2, -3,
        +2, -4, +2, -5, END
    };

    /* Small "user" markers */

    DEFINE_MARKER(um_small_plus, 2) = {
        -1, 0, 1, 0, 0, -1, 0, 1, 0, 0, END
    };

    DEFINE_MARKER(um_small_exclam, 1) = {
        0, -1, 0, 0, 0, 2, END
    };

    DEFINE_MARKER(um_small_times, 1) = {
        0, 0, -1, -1, -1, 1, 1, 1, 1, -1, END
    };

    DEFINE_MARKER(um_small_diamond, 1) = {
        -1, 0, 1, 0, 0, -1, 0, 1, END
    };

    DEFINE_MARKER(um_small_flag, 1) = {
        0, 0, 0, -1, 0, -2, 1, -1, 1, -2, END
    };

    DEFINE_MARKER(um_small_updown, 2) = {
        0, -1, 0, 1, 0, 0, -1, -1, -1, 1, 1, 1, 1, -1, END
    };

    DEFINE_MARKER(um_small_leftright, 1) = {
        0, 0, -1, -1, -1, 1, 1, 1, 1, -1, -1, 0, 1, 0, END
    };

    /* Pointy flags */
    DEFINE_MARKER(pointyflag_1, 1) = {
        0, 0, 0, -1, 0, -2, 0, -3, 0,-4, 0, -5, 0, -6,
        1, -6, 2, -5, 3, -5, 4, -4, 3, -3, 2, -3, 1, -2, END
    };

    DEFINE_MARKER(pointyflag_2, 1) = {
        0, 0, 0, -1, 0, -2, 0, -3,
        1, -3, 2, -2, 1, -1, END
    };

    /* Selection markers ("Andreas cross") */

    DEFINE_MARKER(selection_1, 2) = {
        0, 0, -1, -1, -1, 1, 1, -1, 1, 1, -2, -1, -2, 1, 2, -1, 2, 1, END
    };

    DEFINE_MARKER(selection_2, 4) = {
        0, 0, -1, -1, -1, 1, 1, -1, 1, 1, -2, -1, -2, 1, 2, -1, 2, 1, -3, -2,
        -3, 2, 3, -2, 3, 2, -4, -2, 4, -2, -4, 2, 4, 2, END
    };

    DEFINE_MARKER(selection_3, 6) = {
        0, 0, -1, -1, -1, 1, 1, -1, 1, 1, -2, -1, -2, 1, 2, -1, 2, 1, -3, -2,
        -3, 2, 3, -2, 3, 2, -4, -2, 4, -2, -4, 2, 4, 2, -5, -3, 5, -3, -5, 3,
        5, 3, -6, -3, 6, -3, -6, 3, 6, 3, END
    };

    /* Dotted circles (for starchart). These have been constructed
       so that two concentric circles with difference 1 in radius do
       look good. */

    DEFINE_MARKER(dotted_1, 1) = {
        -1, -1, -1, 1, 1, 1, 1, -1, END
    };

    DEFINE_MARKER(dotted_2, 2) = {
        -2, -1, -1, -2, 1, -2, 2, -1, 2, 1, 1, 2, -1, 2, -2, 1, END
    };

    DEFINE_MARKER(dotted_3, 3) = {
        -3, -1, -3, 1, 3, -1, 3, 1, -1, -3, -1, 3, 1, -3, 1, 3, END
    };

    DEFINE_MARKER(dotted_4, 4) = {
        -4, -2, -4, 0, -4, 2, 4, -2, 4, 0, 4, 2, -2, -4, 0, -4, 2, -4, -2, 4,
        0, 4, 2, 4, END
    };

    DEFINE_MARKER(dotted_5, 5) = {
        -5, -1, -4, -3, -3, -4, -1, -5, 1, -5, 3, -4, 4, -3, 5, -1,
        5, 1, 4, 3, 3, 4, 1, 5, -1, 5, -3, 4, -4, 3, -5, 1, END
    };

    DEFINE_MARKER(dotted_6, 6) = {
        -6, -1, -5, -3, -3, -5, -1, -6, 1, -6, 3, -5, 5, -3, 6, -1,
        6, 1, 5, 3, 3, 5, 1, 6, -1, 6, -3, 5, -5, 3, -6, 1, END
    };

    DEFINE_MARKER(dotted_7, 7) = {
        -7, -1, -6, -3, -5, -5, -3, -6, -1, -7, 1, -7, 3, -6, 5, -5, 6, -3, 7, -1,
        7, 1, 6, 3, 5, 5, 3, 6, 1, 7, -1, 7, -6, 3, -5, 5, -3, 6, -7, 1, END
    };

    /* Triangle ships. Own ships are upwards triangles, foreign ships point down
       (this used to be the other way 'round up to beta 7, but 1.x uses the "new"
       interpretation). */
    DEFINE_MARKER(ship_enemy, 2) = {
        0,+3,  0,+2,  -1,+1,  -1,0,   1,+1, 1,0,  -2,-1,  -2,-2,  -1,-2,  0,-2,
        1,-2, 2,-2, 2,-1, END
    };

    DEFINE_MARKER(ship_own, 2) = {
        0,-3,  0,-2,  -1,-1,  -1,0,   1,-1, 1,0,  -2,1,   -2,2,   -1,2,   0,2,
        1,2,  2,2,  2,1, END
    };

    DEFINE_MARKER(ship_small_enemy, 2) = {
        0,-1, 0,0, 0,1, -1,-1, 1,-1, END
    };

    DEFINE_MARKER(ship_small_own, 2) = {
        -1,1, 1,1, 0,-1, 0,0, 0,1,  END
    };
}

void
client::drawMarker(gfx::BaseContext& ctx, const Marker& marker, gfx::Point pt)
{
    // ex client/marks.h:drawMarker, chart.pas:DrawMarker
    const int8_t* ptr = marker.data;
    const Color_t color = ctx.getRawColor();
    const Memory<const Color_t> colorMem = Memory<const Color_t>::fromSingleObject(color);
    while (*ptr != END) {
        ctx.canvas().drawPixels(pt + gfx::Point(ptr[0], ptr[1]), colorMem, ctx.getAlpha());
        ptr += 2;
    }
}

void
client::drawDottedCircle(gfx::BaseContext& ctx, gfx::Point pt, int r)
{
    // ex client/marks.h:drawDottedCircle, chart.pas:DottedCircle
    static const Marker*const markers[] = {
        &dotted_1, &dotted_2, &dotted_3, &dotted_4,
        &dotted_5, &dotted_6, &dotted_7
    };
    if (r > 0 && r <= int(countof(markers))) {
        drawMarker(ctx, *markers[r-1], pt);
    } else {
        // FIXME: else what?
    }
}

void
client::drawSelection(gfx::BaseContext& ctx, gfx::Point pt, int mult, int divi)
{
    // ex client/marks.h:drawSelection, chart.pas:NDrawSelection
    if (mult <= 0 || divi <= 0) {
        /* invalid, ignore */
    } else if (mult > divi) {
        /* Zoom > 1 */
        int dx = std::min(int(6 * mult / divi), 12);
        int dy = dx/2;
        ctx.setLinePattern(gfx::SOLID_LINE);
        ctx.setLineThickness(1);
        drawLine(ctx, pt + gfx::Point(-dx, -dy), pt + gfx::Point(dx,  dy));
        drawLine(ctx, pt + gfx::Point(-dx,  dy), pt + gfx::Point(dx, -dy));
    } else if (mult*3 > divi*2) {
        /* Zoom > 2/3 */
        drawMarker(ctx, selection_3, pt);
    } else if (divi > mult*2) {
        /* Zoom < 1/2 */
        drawMarker(ctx, selection_1, pt);
    } else {
        drawMarker(ctx, selection_2, pt);
    }
}

void
client::drawMessageMarker(gfx::BaseContext& ctx, const gfx::Point pt, int mult, int divi)
{
    // ex drawFlag
    if (mult > divi) {
        int h = 6 * mult/divi;
        int w = 4 * mult/divi;
        int m = h-w;
        ctx.setLinePattern(gfx::SOLID_LINE);
        ctx.setLineThickness(1);
        drawLine(ctx, pt, pt + gfx::Point(0, -h));
        drawLine(ctx, pt + gfx::Point(0, -h), pt + gfx::Point(w, -w));
        drawLine(ctx, pt + gfx::Point(0, -m), pt + gfx::Point(w, -w));
    } else if (divi > mult*2) {
        /* Zoom < 1/2 */
        drawMarker(ctx, pointyflag_2, pt);
    } else {
        /* Zoom between 1 and 1/2 */
        drawMarker(ctx, pointyflag_1, pt);
    }
}


void
client::drawShipIcon(gfx::BaseContext& ctx, const gfx::Point pt, bool isMe, bool big)
{
    const Marker* p;
    if (isMe) {
        p = big ? &ship_own : &ship_small_own;
    } else {
        p = big ? &ship_enemy : &ship_small_enemy;
    }
    drawMarker(ctx, *p, pt);
}

const client::Marker*
client::getUserMarker(int kind, bool big)
{
    // ex getUserMarker, chart.pas:DrawUserMarker
    static const Marker*const user_markers[][2] = {
        { &um_plus,      &um_small_plus },
        { &um_exclam,    &um_small_exclam },
        { &um_times,     &um_small_times },
        { &um_diamond,   &um_small_diamond },
        { &um_flag,      &um_small_flag },
        { &um_updown,    &um_small_updown },
        { &um_leftright, &um_small_leftright },
        { &um_cactus,    &um_cactus }
    };
    static_assert(countof(user_markers) == size_t(NUM_USER_MARKERS), "user_markers definitions");
    if (kind >= 0 && kind < NUM_USER_MARKERS) {
        return user_markers[kind][!big];
    } else {
        return 0;
    }
}

int
client::getMarkerHeight(const Marker& marker)
{
    return marker.height;
}
