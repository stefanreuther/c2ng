/**
  *  \file client/marker.cpp
  */

#include <algorithm>
#include "client/marker.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/base/countof.hpp"
#include "gfx/complex.hpp"
#include "gfx/canvas.hpp"

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

// /** Draw a Marker.
//     \param ctx     graphics context
//     \param marker  marker definition
//     \param x,y     center  */
void
client::drawMarker(gfx::Context& ctx, const Marker& marker, gfx::Point pt)
{
    // ex client/marks.h:drawMarker
    const int8_t* ptr = marker.data;
    while (*ptr != END) {
        ctx.canvas().drawPixel(pt + gfx::Point(ptr[0], ptr[1]), ctx.getRawColor(), ctx.getAlpha());
        ptr += 2;
    }
}

// /** Draw a dotted circle.
//     \param can graphics context
//     \param x,y center
//     \param r   radius, in range [0, 7]. */
void
client::drawDottedCircle(gfx::Context& ctx, gfx::Point pt, int r)
{
    // ex client/marks.h:drawDottedCircle
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

// /** Draw Selection Marker.
//     \param ctx        graphics context
//     \param pt         center position
//     \param mult,divi  Zoom settings. 1:1 = standard size for big font,
//                       1:2 = size for normal font, others = starchart zoom. */
void
client::drawSelection(gfx::Context& ctx, gfx::Point pt, int mult, int divi)
{
    // ex client/marks.h:drawSelection
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


// void drawShipIcon(gfx::Context& ctx, GPlayerRelation relation, bool big, const GfxPoint pt);
// /** Draw ship icon. This is used in ships-are-triangles mode.
//     \param ctx       graphics context
//     \param relation  our relation to that ship (chooses the marker)
//     \param big       big or small version
//     \param x,y       center position */
// void
// drawShipIcon(GfxContext& ctx, GPlayerRelation relation, bool big, const GfxPoint pt)
// {
//     const TMarkerImage* i;
//     if (relation == is_Me)
//         i = big ? &ship_own : &ship_small_own;
//     else
//         i = big ? &ship_enemy : &ship_small_enemy;

//     drawMarker(ctx, *i, pt);
// }



// /** Get user marker. Returns marker data. Use as
//     drawMarker(can, parm, getUserMarker(id, big), pt). */
const client::Marker*
client::getUserMarker(int id, bool big)
{
    // ex getUserMarker
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
    if (id >= 0 && id < NUM_USER_MARKERS) {
        return user_markers[id][!big];
    } else {
        return 0;
    }
}

