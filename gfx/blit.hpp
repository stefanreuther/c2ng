/**
  *  \file gfx/blit.hpp
  */
#ifndef C2NG_GFX_BLIT_HPP
#define C2NG_GFX_BLIT_HPP

#include "gfx/canvas.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/point.hpp"

namespace gfx {

// /** Simple, generic stretching/shearing/rotating routine. This
//     function blits a transformed version of /src/ onto /target/. The
//     input pixmap, a rectangle, will be mapped to a parallelogram which
//     is given by two vectors.

//     Note that both source and target canvases must be basic surfaces,
//     i.e. not filters. To display a rotated pixmap on a UI canvas, you
//     usually prepare it in a GfxPixmap object and blit that. This will
//     also support proper clipping.

//     \param src         source canvas
//     \param target      target canvas
//     \param src_clip    input rectangle (coordinates in terms of /src/)
//     \param target_clip compute this area of /target/. Can be used to clip
//                        output. Only this area is recomputed, so be sure to
//                        make this big enough. On the other hand, don't make
//                        it too big to not waste time.
//     \param x,y         output origin. src's (0,0) will be transformed to
//                        this position.
//     \param x1,y1       vector to which X axis is mapped
//     \param x2,y2       vector to which Y axis is mapped */
    // FIXME: this function prototype needs to be updated:
    // - replace x,y by Point
    // - swap src/target
    void blitStretchRotate(Canvas& src, Canvas& target, Rectangle src_clip, Rectangle target_clip, int x, int y, int x1, int y1, int x2, int y2);

// /** Compute bounding rectangle for stretch/rotate operation.
//     \param x1,y1  X axis vector
//     \param x2,y2  Y axis vector
//     \return minimal rectangle enclosing the parallelogram defined by the parameters
//     (minimum target_clip for a doStretchRotateBlit() anchored at (0,0). */
    Rectangle computeStretchRotateBBox(int x1, int y1, int x2, int y2);
    
}

#endif
