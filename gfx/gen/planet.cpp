/**
  *  \file gfx/gen/planet.cpp
  *  \brief Class gfx::gen::Planet
  */

#include <cmath>
#include <cassert>
#include "gfx/gen/planet.hpp"
#include "gfx/gen/perlinnoise.hpp"

namespace {
    inline double square(double d)
    {
        return d*d;
    }
}

gfx::gen::Planet::Planet(RGBAPixmap& pix)
    : m_pixmap(pix)
{ }

void
gfx::gen::Planet::renderPlanet(const ValueVector_t planetPos,
                               const Value_t planetRadius,
                               const afl::base::Memory<const ColorQuad_t> terrainColors,
                               const Value_t clearness,
                               const ValueVector_t lightSource,
                               util::RandomNumberGenerator& rng)
{
    // Abuse protection: we need at least 2 colors for terrain.
    if (terrainColors.size() < 2) {
        return;
    }

    // Noise functions
    PerlinNoise terrainNoise(rng);
    PerlinNoise cloudNoise(rng);

    // We must scale the noise functions. It happens that using planetRadius looks good here.
    const Value_t terrainScale = 1.0 / planetRadius;
    const Value_t cloudScale   = 1.0 / planetRadius;

    // Offsets. Their main purpose is to get away from the origin as our noise functions are not wrap-capable.
    const ValueVector_t terrainOffset(10, 10, 10);
    const ValueVector_t cloudOffset(20, 20, 20);

    // Determine area of render
    const int32_t minX = std::max(int32_t(planetPos.x - planetRadius - 1), int32_t(0));
    const int32_t maxX = std::min(int32_t(planetPos.x + planetRadius + 1), int32_t(m_pixmap.getWidth()));
    const int32_t minY = std::max(int32_t(planetPos.y - planetRadius - 1), int32_t(0));
    const int32_t maxY = std::min(int32_t(planetPos.y + planetRadius + 1), int32_t(m_pixmap.getHeight()));

    // Render
    const Value_t numTerrainColors = Value_t(terrainColors.size() - 1);
    for (int y = minY; y < maxY; ++y) {
        for (int x = minX; x < maxX; ++x) {
            // Planet surface
            ValueVector_t surface;
            Value_t c = calcLight(planetPos, planetRadius, lightSource, ValueVector_t(x, y, 0), surface);
            if (c >= 0) {
                // Compute terrain color: noise function selects from color gradient.
                const Value_t terrain = recursiveField(terrainNoise, terrainOffset + surface*terrainScale, 5, 1.5);
                const Value_t tsel    = std::max(Value_t(0), std::min(numTerrainColors, terrain * numTerrainColors));
                const ColorQuad_t c1  = *terrainColors.at(int(tsel));
                const ColorQuad_t c2  = *terrainColors.at(int(tsel)+1);
                const Value_t w       = tsel - int(tsel);
                ColorQuad_t color     = mixColor(c1, c2, uint8_t(255*w));

                // Add cloud color: noise function selects cloud density. Only (1/clearness) of the sky has clouds.
                Value_t cloud = std::max(Value_t(0), recursiveField(cloudNoise, cloudOffset + surface*cloudScale, 5, 3)) * clearness;
                if (cloud < 1) {
                    color = mixColor(color, COLORQUAD_FROM_RGBA(255, 255, 255, TRANSPARENT_ALPHA), uint8_t(255*(1-cloud)));
                }

                // Adjust according to lighting
                color = mixColor(color, COLORQUAD_FROM_RGBA(0, 0, 0, OPAQUE_ALPHA), uint8_t(255*c));

                // Make fully opaque
                color |= COLORQUAD_FROM_RGBA(0, 0, 0, OPAQUE_ALPHA);

                // Store pixel
                ColorQuad_t* pPixel = m_pixmap.row(y).at(x);
                assert(pPixel != 0);
                *pPixel = color;
            }
        }
    }
}


gfx::gen::Planet::Value_t
gfx::gen::Planet::recursiveField(PerlinNoise& pn, const ValueVector_t& v, int32_t depth, Value_t mult)
{
    if (depth <= 0) {
        return pn.noise(v.x * mult, v.y * mult, v.z * mult);
    } else {
        Value_t displace = recursiveField(pn, v, depth-1, mult * 2);
        return pn.noise(v.x * mult + displace, v.y * mult + displace, v.z * mult + displace);
    }
}

/** Compute light.
    We are using an orthographic camera looking along the Z axis, at the planet which lies on the X/Y plane.
    Likewise, the light source looks at the same planet, from the given point.

    We need to determine:
    - whether the camera actually hits the planet
    - the angle at which the light source hits the planet surface

    That angle gives an estimate of how lit the planet surface is. */
inline gfx::gen::Planet::Value_t
gfx::gen::Planet::calcLight(const ValueVector_t& planet,
                            Value_t planetRadius,
                            const ValueVector_t& light,
                            const ValueVector_t& camera,
                            ValueVector_t& surface)
{
    // Compute d: vector where camera hits planet surface
    ValueVector_t d = camera - planet;
    Value_t z2 = square(planetRadius) - square(d.x) - square(d.y);
    if (z2 < 0) {
        return -1;
    }
    d.z = std::sqrt(z2);
    surface = d;

    // Compute L: vector from light source to planet center
    ValueVector_t L = light - planet;

    // Angle between d and L:
    //    dot(L,d)      = mag(L)*mag(d)*cos(phi)
    // Note that mag(d) = planetRadius.
    Value_t dot    = d.dot(L);
    Value_t dmag   = planetRadius;
    Value_t Lmag   = std::sqrt(L.mag2());
    Value_t cosphi = dot / (dmag*Lmag);

    // Transform into [0,1] range.
    // Instead of some cool physical reasoning, I chose a formula that happens to look good.
    return square(cosphi+1)/4;
}
