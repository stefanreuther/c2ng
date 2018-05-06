/**
  *  \file gfx/gen/orbitconfig.hpp
  *  \brief Class gfx::gen::OrbitConfig
  */
#ifndef C2NG_GFX_GEN_ORBITCONFIG_HPP
#define C2NG_GFX_GEN_ORBITCONFIG_HPP

#include "gfx/rgbapixmap.hpp"
#include "gfx/point.hpp"
#include "util/randomnumbergenerator.hpp"

namespace gfx { namespace gen {

    /** Orbit view renderer, configuration.
        An orbit view combines a space view and a planet. */
    class OrbitConfig {
     public:
        /** Constructor. */
        OrbitConfig();

        /** Set image size.
            Images will be scaled, that is, requesting a double-size image will
            produce (mostly) the same content at higher resolution.
            \param pt Size (default: 640x480) */
        void setSize(Point pt);

        /** Set number of stars (far stars).
            \param n Number (default: 5) */
        void setNumStars(int n);

        /** Set relative position of planet center.
            Note that the default configuration is (100,500), i.e. the planet is outside the frame.
            \param relX Relative X position, percentage (0=left, 50=center, 100=right)
            \param relY Relative Y position, percentage (0=top, 50=center, 100=bottom) */
        void setPlanetPosition(int relX, int relY);

        /** Set relative planet radius.
            Note that the default configuration is 415, i.e. the planet is larger than the frame (and outside, see setPlanetPosition).
            \param relRadius Relative radius (100=same as minimum image dimension, i.e. completely fills frame) */
        void setPlanetRadius(int relRadius);

        /** Render.
            Produces an image using the given settings.
            \param rng Random number generator
            \return New image */
        afl::base::Ref<RGBAPixmap> render(util::RandomNumberGenerator& rng) const;

     private:
        int m_width;
        int m_height;
        int m_numStars;
        int m_planetRelX;
        int m_planetRelY;
        int m_planetRelRadius;
    };

} }

#endif
