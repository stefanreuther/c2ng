/**
  *  \file gfx/colortransform.hpp
  *  \brief Color Transformation
  */
#ifndef C2NG_GFX_COLORTRANSFORM_HPP
#define C2NG_GFX_COLORTRANSFORM_HPP

#include "gfx/canvas.hpp"
#include "gfx/types.hpp"

namespace gfx {

    /** Convert canvas to monochrome.
        Creates a new canvas of identical dimensions and similar color mode as the provided one,
        and transforms colors to a monochrome.
        The given color parameter specifies the target color; a white pixel will have this color.

        @param in    Source canvas
        @param color Color
        @return newly-created canvas with transformed image */
    afl::base::Ref<Canvas> convertToMonochrome(Canvas& in, ColorQuad_t color);

}

#endif
