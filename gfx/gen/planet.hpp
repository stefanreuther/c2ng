/**
  *  \file gfx/gen/planet.hpp
  *  \brief Class gfx::gen::Planet
  */
#ifndef C2NG_GFX_GEN_PLANET_HPP
#define C2NG_GFX_GEN_PLANET_HPP

#include "afl/base/memory.hpp"
#include "gfx/gen/vector3d.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/types.hpp"
#include "util/randomnumbergenerator.hpp"

namespace gfx { namespace gen {

    class PerlinNoise;

    /** Planet renderer.
        Allows you to render single planets. */
    class Planet {
     public:
        /** Value. */
        typedef double Value_t;

        /** 3-D vector of values (point in space). */
        typedef Vector3D<Value_t> ValueVector_t;

        /** Constructor.
            \param pix Output pixmap */
        explicit Planet(RGBAPixmap& pix);

        /** Render a planet.
            \param planetPos     [in] Planet position, in image coordinates
            \param planetRadius  [in] Planet radius, in image coordinates
            \param terrainColors [in] Colors to build the planet terrain from (at least 2 colors)
            \param clearness     [in] Cloud clearness: 1=totally clouded, >1: less clouded
            \param lightSource   [in] Position of light source (sun) in image coordinates
            \param rng           [in/out] Random number generator */
        void renderPlanet(ValueVector_t planetPos,
                          Value_t planetRadius,
                          afl::base::Memory<const ColorQuad_t> terrainColors,
                          Value_t clearness,
                          ValueVector_t lightSource,
                          util::RandomNumberGenerator& rng);

     private:
        RGBAPixmap& m_pixmap;

        static Value_t recursiveField(PerlinNoise& pn, const ValueVector_t& v, int32_t depth, Value_t mult);
        static Value_t calcLight(const ValueVector_t& planet, Value_t planetRadius, const ValueVector_t& light, const ValueVector_t& camera, ValueVector_t& surface);
    };

} }

#endif
