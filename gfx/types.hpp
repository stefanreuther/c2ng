/**
  *  \file gfx/types.hpp
  */
#ifndef C2NG_GFX_TYPES_HPP
#define C2NG_GFX_TYPES_HPP

#include "afl/base/types.hpp"

namespace gfx {

    /** Color. This is an opaque value. */
    typedef uint32_t Color_t;

    /** Color quad. */
    typedef uint32_t ColorQuad_t;

#define COLORQUAD_FROM_RGBA(r,g,b,a)  ::gfx::ColorQuad_t(((r)<<24) + ((g)<<16) + ((b)<<8) + (a))
#define COLORQUAD_FROM_RGB(r,g,b)     COLORQUAD_FROM_RGBA(r, g, b, ::gfx::OPAQUE_ALPHA)
#define RED_FROM_COLORQUAD(c)         uint8_t(((c)>>24) & 0xFF)
#define GREEN_FROM_COLORQUAD(c)       uint8_t(((c)>>16) & 0xFF)
#define BLUE_FROM_COLORQUAD(c)        uint8_t(((c)>>8) & 0xFF)
#define ALPHA_FROM_COLORQUAD(c)       uint8_t((c) & 0xFF)

    /** Transparent color.
        When a background color is requested, specify this value to draw transparent.
        This value has been chosen to be a very unlikely if not impossible color value. */
    const Color_t TRANSPARENT_COLOR = 0xFFFFFFFEU;



    /** Type for alpha channel. */
    typedef uint8_t Alpha_t;

    const Alpha_t OPAQUE_ALPHA = 255;
    const Alpha_t TRANSPARENT_ALPHA = 0;



    /** Line patterns. */
    typedef uint8_t LinePattern_t;

    const LinePattern_t SOLID_LINE  = 0xFF;
    const LinePattern_t DASHED_LINE = 0xF0;
    const LinePattern_t DOTTED_LINE = 0xAA;

    /*
     *  Operations
     */

    /** Alpha mixing for a single color component.
        \tparam T     value type
        \param a      background color
        \param b      color to write
        \param alpha  alpha value of color to write. */
    template<class T> T mixColorComponent(T a, T b, Alpha_t alpha);

    /** Alpha mixing for a ColorQuad_t.
        \param a      background color
        \param b      color to write
        \param alpha  alpha value of color to write. */
    ColorQuad_t mixColor(ColorQuad_t a, ColorQuad_t b, Alpha_t alpha);

    /** Adding two ColorQuad_t.
        \param a      color 1
        \param b      color 2. */
    ColorQuad_t addColor(ColorQuad_t a, ColorQuad_t b);

    /** Get distance metric between two colors.
        \param x,y Colors
        \return distance metric (lower is closer, 0 if x==y) */
    int32_t getColorDistance(gfx::ColorQuad_t x, gfx::ColorQuad_t y);

}


template<class T>
inline T
gfx::mixColorComponent(T a, T b, Alpha_t alpha)
{
    return T(a + ((b-a) * alpha / 255));
}

#endif
