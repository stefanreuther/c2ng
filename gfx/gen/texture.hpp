/**
  *  \file gfx/gen/texture.hpp
  */
#ifndef C2NG_GFX_GEN_TEXTURE_HPP
#define C2NG_GFX_GEN_TEXTURE_HPP

#include "gfx/rgbapixmap.hpp"
#include "util/randomnumbergenerator.hpp"
#include "gfx/gen/colorrange.hpp"

namespace gfx { namespace gen {

    class Texture {
     public:
        explicit Texture(RGBAPixmap& pix);

        void fill(ColorQuad_t color);

        void fillNoise(ColorRange r, util::RandomNumberGenerator& rng);

        void renderCircularGradient(Point center, int radius, ColorRange range, util::RandomNumberGenerator& rng, int noiseScale);

        void renderBrush(ColorRange r, int count, int angle, util::RandomNumberGenerator& rng);

        RGBAPixmap& pixmap();

     private:
        RGBAPixmap& m_pixmap;
    };

} }

#endif
