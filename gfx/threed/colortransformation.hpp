/**
  *  \file gfx/threed/colortransformation.hpp
  *  \brief Class gfx::threed::ColorTransformation
  */
#ifndef C2NG_GFX_THREED_COLORTRANSFORMATION_HPP
#define C2NG_GFX_THREED_COLORTRANSFORMATION_HPP

#include "gfx/threed/vecmath.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace threed {

    /** Color transformation, represented as a matrix.
        A ColorTransformation can be
        - brightness adjustment
        - adding another color (=mixing)
        - conversion to grayscale or any other linear combination of components

        Each row describes a component:
        - r_out = m(0) * r_in + m(1) * g_in + m(2) * b_in + m(3)
        - g_out = m(4) * r_in + m(5) * g_in + m(6) * b_in + m(7)
        - b_out = m(8) * r_in + m(9) * g_in + m(10) * b_in + m(11)
        The fourth row is unused and must always remain at (1,0,0,0).

        Colors are transformed using `tr.transform(color)`,
        offering both a version using a `Vec3f` or a `ColorQuad_t`.
        A `Vec3f` can contain values between 0.0 and 1.0; out-of-range values are preserved.
        A `ColorQuad_t` can contain values between 0 and 255; out-of-ranged values are clamped into range.
        Whereas `ColorQuad_t` can contain an alpha channel, `Vec3f` can not.

        To combine two transformation, matrix-multiply them, i.e.
        `then.transform(first.transform(x)) == (first * then).transform(x)`.

        Note that the order of coefficients in transformation,
        and thus the order of operands in matrix multiplication,
        differs from the operations used for coordinate transformations (Vec3/Mat4). */
    class ColorTransformation : public detail::Mat<float, double, 4, ColorTransformation> {
     public:
        /** Coefficients for toGrayscale: Rec. 601. */
        static const Vec3f GRAYSCALE_REC601;

        /** Coefficients for toGrayscale: ITU-R BT.709 / SRGB. */
        static const Vec3f GRAYSCALE_BT709;

        /** Coefficients for toGrayscale: ITU-R BT.2100. */
        static const Vec3f GRAYSCALE_BT2100;

        /** Coefficients for toGrayscale: overly simple.
            Just take the unweighed average of R,G,B. */
        static const Vec3f GRAYSCALE_SIMPLE;

        /** Construct from 16 values.
            @param a1,a2,a3,a4  Coefficients for computing red component
            @param b1,b2,b3,b4  Coefficients for computing green component
            @param c1,c2,c3,c4  Coefficients for computing blue component
            @param d1,d2,d3,d4  Must be 0,0,0,1 */
        ColorTransformation(float a1, float a2, float a3, float a4,
                            float b1, float b2, float b3, float b4,
                            float c1, float c2, float c3, float c4,
                            float d1, float d2, float d3, float d4);

        /** Make identity transformation.
            @return identity transformation */
        static ColorTransformation identity();

        /** Make conversion to grayscale, general version.
            @param color   Result color. Fully-white input produces this color on output.
            @param weights Weights to be given to R/G/B components. Parts should add to 1.
                           See GRAYSCALE_xxx constants. */
        static ColorTransformation toGrayscale(ColorQuad_t color, Vec3f weights);

        /** Make conversion to grayscale.
            Uses GRAYSCALE_REC601 factors.
            @param color   Result color. Fully-white input produces this color on output. */
        static ColorTransformation toGrayscale(ColorQuad_t color);

        /** Scale (adjust brightness).
            Updates the transformation in-place.
            @param factor Factor to adjust by (>1: make brighter, <1: dim).
            @return *this */
        ColorTransformation& scale(float factor);

        /** Add (mix in color), vector version.
            Updates the transformation in-place.
            @param vec Color as Vec3f [0.0, 1.0]
            @return *this */
        ColorTransformation& add(const Vec3f& vec);

        /** Add (mix in color), color version.
            Updates the transformation in-place.
            @param vec Color
            @return *this */
        ColorTransformation& add(ColorQuad_t color);

        /** Matrix multiplication, in-place.
            Updates this transformation to perform the other transformation as well.
            @param other Matrix to multiply by
            @return *this */
        ColorTransformation& operator*=(const ColorTransformation& other);

        /** Matrix multiplication.
            Returns transformation that first executes this, then other.
            @param other Matrix to multiply by
            @return product */
        ColorTransformation operator*(const ColorTransformation& other) const;

        /** Matrix addition, in-place.
            Updates this transformation to perform both transformations and mix (add) the results.
            @param other Matrix to add
            @return *this */
        ColorTransformation& operator+=(const ColorTransformation& other);

        /** Matrix addition.
            Returns transformation that performs both transformations and mixes (adds) the results.
            @param other Matrix to multiply by
            @return product */
        ColorTransformation operator+(const ColorTransformation& other) const;

        /** Color transformation, vector version.
            @param vec Input color
            @return Result color */
        Vec3f transform(const Vec3f& vec) const;

        /** Color transformation.
            Alpha is preserved from input to output.
            @param color Input color
            @return color */
        ColorQuad_t transform(ColorQuad_t color) const;
    };

    /** Convert Vec3f into ColorQuad_t.
        @param vec Input
        @param alpha Alpha to use
        @return result */
    ColorQuad_t fromFloat(const Vec3f& vec, uint8_t alpha);

    /** Convert ColorQuad_t to Vec3f.
        @param color Input
        @return result */
    Vec3f toFloat(ColorQuad_t color);

} }

#endif
