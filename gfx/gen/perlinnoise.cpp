/**
  *  \file gfx/gen/perlinnoise.cpp
  *  \brief Class gfx::gen::PerlinNoise
  *
  *  Derived from procedural.js, see spaceview.cpp for details.
  */

#include "gfx/gen/perlinnoise.hpp"

const gfx::gen::PerlinNoise::Triplet_t gfx::gen::PerlinNoise::grad3[] = {
    { 1, 1, 0 },
    { -1, 1, 0 },
    { 1, -1, 0 },
    { -1, -1, 0 },
    { 1, 0, 1 },
    { -1, 0, 1 },
    { 1, 0, -1 },
    { -1, 0, -1 },
    { 0, 1, 1 },
    { 0, -1, 1 },
    { 0, 1, -1 },
    { 0, -1, -1 },
};

// Constructor.
gfx::gen::PerlinNoise::PerlinNoise(util::RandomNumberGenerator& rng)
{
    for (size_t i = 0; i < 256; ++i) {
        uint8_t value = static_cast<uint8_t>(rng(256));
        perm[i + 256] = perm[i] = value;
        perm12[i + 256] = perm12[i] = value % 12;
    }
}

// Compute 3-D noise value.
gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::noise(Value_t x, Value_t y, Value_t z) const
{
    int32_t X = int32_t(x);
    int32_t Y = int32_t(y);
    int32_t Z = int32_t(z);
    x = x - X;
    y = y - Y;
    z = z - Z;
    X = X & 255;
    Y = Y & 255;
    Z = Z & 255;

    int32_t gi000 = perm12[X +     perm[Y +     perm[Z]]]    ;
    int32_t gi001 = perm12[X +     perm[Y +     perm[Z + 1]]];
    int32_t gi010 = perm12[X +     perm[Y + 1 + perm[Z]]]    ;
    int32_t gi011 = perm12[X +     perm[Y + 1 + perm[Z + 1]]];
    int32_t gi100 = perm12[X + 1 + perm[Y +     perm[Z]]]    ;
    int32_t gi101 = perm12[X + 1 + perm[Y +     perm[Z + 1]]];
    int32_t gi110 = perm12[X + 1 + perm[Y + 1 + perm[Z]]]    ;
    int32_t gi111 = perm12[X + 1 + perm[Y + 1 + perm[Z + 1]]];

    Value_t n000 = dot(grad3[gi000], x,     y,     z);
    Value_t n100 = dot(grad3[gi100], x - 1, y,     z);
    Value_t n010 = dot(grad3[gi010], x,     y - 1, z);
    Value_t n110 = dot(grad3[gi110], x - 1, y - 1, z);
    Value_t n001 = dot(grad3[gi001], x,     y,     z - 1);
    Value_t n101 = dot(grad3[gi101], x - 1, y,     z - 1);
    Value_t n011 = dot(grad3[gi011], x,     y - 1, z - 1);
    Value_t n111 = dot(grad3[gi111], x - 1, y - 1, z - 1);

    Value_t u = fade(x);
    Value_t v = fade(y);
    Value_t w = fade(z);
    Value_t nx00 = mix(n000, n100, u);
    Value_t nx01 = mix(n001, n101, u);
    Value_t nx10 = mix(n010, n110, u);
    Value_t nx11 = mix(n011, n111, u);
    Value_t nxy0 = mix(nx00, nx10, v);
    Value_t nxy1 = mix(nx01, nx11, v);
    Value_t nxyz = mix(nxy0, nxy1, w);

    return 0.5 * nxyz + 0.5;
}

// Compute 2-D noise value.
gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::noise(Value_t x, Value_t y) const
{
    int32_t X = int32_t(x);
    int32_t Y = int32_t(y);

    x = x - X;
    y = y - Y;

    X = X & 255;
    Y = Y & 255;

    int32_t gi000 = perm12[X +     perm[Y +     perm[0]]];
    int32_t gi010 = perm12[X +     perm[Y + 1 + perm[0]]];
    int32_t gi100 = perm12[X + 1 + perm[Y +     perm[0]]];
    int32_t gi110 = perm12[X + 1 + perm[Y + 1 + perm[0]]];

    Value_t n000 = dot(grad3[gi000], x,     y);
    Value_t n100 = dot(grad3[gi100], x - 1, y);
    Value_t n010 = dot(grad3[gi010], x,     y - 1);
    Value_t n110 = dot(grad3[gi110], x - 1, y - 1);

    Value_t u = fade(x);
    Value_t v = fade(y);
    Value_t nx00 = mix(n000, n100, u);
    Value_t nx10 = mix(n010, n110, u);
    Value_t nxy0 = mix(nx00, nx10, v);

    return 0.5 * nxy0 + 0.5;
}

inline gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::dot(const Triplet_t& g, Value_t x, Value_t y, Value_t z)
{
    return g[0] * x + g[1] * y + g[2] * z;
}

inline gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::dot(const Triplet_t& g, Value_t x, Value_t y)
{
    return g[0] * x + g[1] * y;
}

inline gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::mix(Value_t a, Value_t b, Value_t t)
{
    return (1.0 - t) * a + t * b;
}

inline gfx::gen::PerlinNoise::Value_t
gfx::gen::PerlinNoise::fade(Value_t t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
