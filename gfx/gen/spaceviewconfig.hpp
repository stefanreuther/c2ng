/**
  *  \file gfx/gen/spaceviewconfig.hpp
  *  \brief Class gfx::gen::SpaceViewConfig
  */
#ifndef C2NG_GFX_GEN_SPACEVIEWCONFIG_HPP
#define C2NG_GFX_GEN_SPACEVIEWCONFIG_HPP

#include "afl/base/ref.hpp"
#include "gfx/rgbapixmap.hpp"
#include "util/randomnumbergenerator.hpp"
#include "gfx/point.hpp"

namespace gfx { namespace gen {

    /** Space View Renderer, Configuration.
        Allows to set a configuration and obtain a ready-made space view image. */
    class SpaceViewConfig {
     public:
        /** Constructor. */
        SpaceViewConfig();

        /** Set image size.
            Images will be scaled, that is, requesting a double-size image will
            produce (mostly) the same content at higher resolution.
            \param pt Size (default: 640x480) */
        void setSize(Point pt);

        /** Set number of suns (close stars).
            \param n Number (default: 1) */
        void setNumSuns(int n);

        /** Set probability of stars.
            This is a percentage.
            A die is rolled repeatedly; this percentage gives the probability that a star is added and the process repeats.
            \param n Percentage (default: 95) */
        void setStarProbability(int n);

        /** Render.
            Produces an image using the given settings.
            \param rng Random number generator
            \return New image */
        afl::base::Ref<RGBAPixmap> render(util::RandomNumberGenerator& rng) const;

     private:
        int m_width;
        int m_height;
        int m_numSuns;
        int m_starProbability;
    };

} }

#endif
