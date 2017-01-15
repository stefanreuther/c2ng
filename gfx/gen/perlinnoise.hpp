/**
  *  \file gfx/gen/perlinnoise.hpp
  *  \brief Class gfx::gen::PerlinNoise
  */
#ifndef C2NG_GFX_GEN_PERLINNOISE_HPP
#define C2NG_GFX_GEN_PERLINNOISE_HPP

#include "util/randomnumbergenerator.hpp"
#include "afl/base/types.hpp"

namespace gfx { namespace gen {

    /** Perlin noise generator.
        Perlin noise is continuous noise that can be computed for floating-point values and produces continuous results.
        This implementation provides 3-D and 2-D noise. */
    class PerlinNoise {
     public:
        typedef double Value_t;

        /** Constructor.
            \param rng [in/out] Random number generator. Required for initialisation only. */
        explicit PerlinNoise(util::RandomNumberGenerator& rng);

        /** Compute 3-D noise value.
            \param x,y,z Coordinates
            \return Noise value */
        Value_t noise(Value_t x, Value_t y, Value_t z) const;

        /** Compute 2-D noise value.
            Returns the same value as noise(x,y,0).
            \param x,y Coordinates
            \return Noise value */
        Value_t noise(Value_t x, Value_t y) const;

     private:
        uint8_t perm[512];
        uint8_t perm12[512];

        typedef Value_t Triplet_t[3];

        static const Triplet_t grad3[12];
        static Value_t dot(const Triplet_t& g, Value_t x, Value_t y, Value_t z);
        static Value_t dot(const Triplet_t& g, Value_t x, Value_t y);
        static Value_t mix(Value_t a, Value_t b, Value_t t);
        static Value_t fade(Value_t t);
    };

} }

#endif
