/**
  *  \file gfx/gen/spaceview.cpp
  *  \brief Class gfx::gen::SpaceView
  *
  *  Heavily inspired by http://wwwtyro.github.io/procedural.js/space/.
  *  That program comes with the following license:
  *
  *  The following applies to both procedural.js and the content it produces.
  *
  *  In a nutshell, it's public domain. No attribution is required, either.
  *
  *  This is free and unencumbered software released into the public domain.
  *
  *  Anyone is free to copy, modify, publish, use, compile, sell, or
  *  distribute this software, either in source code form or as a compiled
  *  binary, for any purpose, commercial or non-commercial, and by any
  *  means.
  *
  *  In jurisdictions that recognize copyright laws, the author or authors
  *  of this software dedicate any and all copyright interest in the
  *  software to the public domain. We make this dedication for the benefit
  *  of the public at large and to the detriment of our heirs and
  *  successors. We intend this dedication to be an overt act of
  *  relinquishment in perpetuity of all present and future rights to this
  *  software under copyright law.
  *
  *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  *  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  *  OTHER DEALINGS IN THE SOFTWARE.
  *
  *  For more information, please refer to <http://unlicense.org>
  */

#include <cmath>
#include <algorithm>
#include "gfx/gen/spaceview.hpp"
#include "gfx/gen/perlinnoise.hpp"
#include "util/math.hpp"

namespace {
    inline void put(gfx::RGBAPixmap& pix, int x, int y, gfx::ColorQuad_t color)
    {
        if (gfx::ColorQuad_t* p = pix.row(y).at(x)) {
            *p = gfx::mixColor(*p, color, ALPHA_FROM_COLORQUAD(color));
        }
    }

    inline void add(gfx::RGBAPixmap& pix, int x, int y, gfx::ColorQuad_t color)
    {
        if (gfx::ColorQuad_t* p = pix.row(y).at(x)) {
            *p = gfx::addColor(*p, color);
        }
    }
}

// Constructor.
gfx::gen::SpaceView::SpaceView(RGBAPixmap& pix)
    : m_pixmap(pix)
{ }

// Render starfield (far stars).
void
gfx::gen::SpaceView::renderStarfield(util::RandomNumberGenerator& rng)
{
    const int w = m_pixmap.getWidth();
    const int h = m_pixmap.getHeight();
    const int numStars = w * h / 512;
    for (int i = 0; i < numStars; ++i) {
        int x = rng(uint16_t(w));
        int y = rng(uint16_t(h));
        int c = rng(256);
        put(m_pixmap, x, y, COLORQUAD_FROM_RGBA(255, 255, 255, (c*c*c) >> 16));
    }
}

// Render star (not so far star, with small halo).
void
gfx::gen::SpaceView::renderStar(ColorQuad_t color, Point pos, Value_t size)
{
    const Value_t e = 0.5;
    const Value_t E = e * 2;
    const Value_t m = std::pow(size, E);

    int side = 0;
    while (m / std::pow(side*side, e + (side*side) / 10000.0) > 0.001) {
        ++side;
    }

    for (int xx = -side; xx < side; ++xx) {
        for (int yy = -side; yy < side; ++yy) {
            Value_t d = xx*xx + yy*yy;
            int i = int(255 * std::min(Value_t(1), m / std::pow(d, e + d / 10000)));
            put(m_pixmap, pos.getX() + xx, pos.getY() + yy, color + COLORQUAD_FROM_RGBA(0, 0, 0, i));
        }
    }
}

// Render nebula.
void
gfx::gen::SpaceView::renderNebula(util::RandomNumberGenerator& rng, ColorQuad_t color, Value_t scale, Value_t intensity, Value_t falloff)
{
    PerlinNoise pn(rng);

    const int width = m_pixmap.getWidth();
    const int height = m_pixmap.getHeight();
    const Value_t nscale = 1.0 / scale;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            put(m_pixmap, x, y, field(pn, color, x * nscale, y * nscale, intensity, falloff));
        }
    }
}

// Render sun (close star).
void
gfx::gen::SpaceView::renderSun(ColorQuad_t color, Point pos, int size)
{
    const Value_t e = 1;
    const Value_t m = std::pow(size, e*2);
    const int width = m_pixmap.getWidth();
    const int height = m_pixmap.getHeight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Value_t d = util::squareInteger(x - pos.getX()) + util::squareInteger(y - pos.getY());
            const Value_t raw = m / std::pow(d, e);
            const Value_t i = std::min(Value_t(1.0), raw);
            const Value_t q = raw - i;

            add(m_pixmap, x, y, COLORQUAD_FROM_RGBA(uint8_t(i * std::min(Value_t(255), RED_FROM_COLORQUAD  (color) + q*2*255)),
                                                    uint8_t(i * std::min(Value_t(255), GREEN_FROM_COLORQUAD(color) + q*4*255)),
                                                    uint8_t(i * std::min(Value_t(255), BLUE_FROM_COLORQUAD (color) + q*2*255)),
                                                    255));
        }
    }
}

gfx::gen::SpaceView::Value_t
gfx::gen::SpaceView::recursiveField(PerlinNoise& pn, Value_t x, Value_t y, int32_t depth, Value_t mult)
{
    if (depth <= 0) {
        return pn.noise(x * mult, y * mult);
    } else {
        Value_t displace = recursiveField(pn, x, y, depth-1, mult * 2);
        return pn.noise(x * mult + displace, y * mult + displace);
    }
}

inline gfx::ColorQuad_t
gfx::gen::SpaceView::field(PerlinNoise& pn, ColorQuad_t rgb, Value_t x, Value_t y, Value_t intensity, Value_t falloff)
{
    Value_t i = std::min(Value_t(1.0), recursiveField(pn, x, y, 5, 0.5) * intensity);
    i = std::pow(i, falloff);
    return rgb + COLORQUAD_FROM_RGBA(0, 0, 0, uint8_t(i * 255));
}
