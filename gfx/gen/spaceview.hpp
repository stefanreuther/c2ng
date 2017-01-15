/**
  *  \file gfx/gen/spaceview.hpp
  *  \brief Class gfx::gen::SpaceView
  */
#ifndef C2NG_GFX_GEN_SPACEVIEW_HPP
#define C2NG_GFX_GEN_SPACEVIEW_HPP

#include "gfx/rgbapixmap.hpp"
#include "util/randomnumbergenerator.hpp"

namespace gfx { namespace gen {

    class PerlinNoise;

    /** Space View Renderer.
        Allows you to render various spacey things.
        You can call the methods in any order, any number of times.
        Each element will be rendered atop the previous ones. */
    class SpaceView {
     public:
        typedef double Value_t;

        /** Constructor.
            \param pix Output pixmap */
        explicit SpaceView(RGBAPixmap& pix);

        /** Render starfield (far stars).
            This just renders a number of single-dot stars.
            \param rng [in/out] random number generator */
        void renderStarfield(util::RandomNumberGenerator& rng);

        /** Render star (not so far star, with small halo).
            \param color Star color. Must have ALPHA_FROM_COLORQUAD(color) == 0.
            \param pos   Position in pixmap
            \param size  Size of star (magnitude of a few 1000th of the image dimensions) */
        void renderStar(ColorQuad_t color, Point pos, Value_t size);

        /** Render nebula.
            \param rng       [in/out] Random number generator
            \param color     [in] Color of nebula
            \param scale     [in] Scale factor (magnitude of image dimension)
            \param intensity [in] Intensity (defines opacity)
            \param falloff   [in] Fall-off (defines opacity) */
        void renderNebula(util::RandomNumberGenerator& rng, ColorQuad_t color, Value_t scale, Value_t intensity, Value_t falloff);

        /** Render sun (close star).
            \param color Color
            \param pos Position in pixmap
            \param size Size in pixels */
        void renderSun(ColorQuad_t color, Point pos, int size);

     private:
        RGBAPixmap& m_pixmap;

        static Value_t recursiveField(PerlinNoise& pn, Value_t x, Value_t y, int32_t depth, Value_t mult);
        static ColorQuad_t field(PerlinNoise& pn, ColorQuad_t rgb, Value_t x, Value_t y, Value_t intensity, Value_t falloff);
    };

} }

#endif
