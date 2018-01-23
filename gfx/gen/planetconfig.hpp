/**
  *  \file gfx/gen/planetconfig.hpp
  *  \brief Class gfx::gen::PlanetConfig
  */
#ifndef C2NG_GFX_GEN_PLANETCONFIG_HPP
#define C2NG_GFX_GEN_PLANETCONFIG_HPP

#include "afl/base/ref.hpp"
#include "gfx/point.hpp"
#include "gfx/rgbapixmap.hpp"
#include "util/randomnumbergenerator.hpp"
#include "gfx/gen/planetconfig.hpp"

namespace gfx { namespace gen {

    /** Planet Renderer, Configuration.
        Allows to set a configuration and obtain a ready-made planet image. */
    class PlanetConfig {
     public:
        /** Constructor.
            Sets up a default configuration. */
        PlanetConfig();

        /** Set image size.
            Images will be scaled, that is, requesting a double-size image will
            produce (mostly) the same content at higher resolution.
            \param pt Size (default: 640x480) */
        void setSize(Point pt);

        /** Set relative position of planet center.
            \param relX Relative X position, percentage (0=left, 50=center, 100=right)
            \param relY Relative Y position, percentage (0=top, 50=center, 100=bottom) */
        void setPlanetPosition(int relX, int relY);

        /** Set relative planet radius.
            \param relRadius Relative radius (100=same as minimum image dimension, i.e. completely fills frame) */
        void setPlanetRadius(int relRadius);

        /** Set planet temperature.
            \param temp Temperature (0-100). */
        void setPlanetTemperature(int temp);

        /** Set sun position.
            \param relX Relative X position, percentage (0=left, 50=center, 100=right)
            \param relY Relative Y position, percentage (0=top, 50=center, 100=bottom)
            \param relZ Relative Z position (positive: behind camera) */
        void setSunPosition(int relX, int relY, int relZ);

        /** Render.
            Produces an image using the given settings.
            \param rng Random number generator
            \return New image */
        afl::base::Ref<RGBAPixmap> render(util::RandomNumberGenerator& rng) const;

     private:
        int m_width;
        int m_height;
        int m_planetRelX;
        int m_planetRelY;
        int m_planetRelRadius;
        int m_planetTemperature;
        int m_sunRelX;
        int m_sunRelY;
        int m_sunRelZ;
    };

} }

#endif
