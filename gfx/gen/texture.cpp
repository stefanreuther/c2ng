/**
  *  \file gfx/gen/texture.cpp
  */

#include <cmath>
#include "gfx/gen/texture.hpp"
#include "util/math.hpp"

namespace {
    void advance(int& fract, int delta, int& num, int limit)
    {
        fract += delta;
        if (fract >= 256) {
            ++num;
            if (num >= limit) {
                num -= limit;
            }
            fract -= 256;
        }
        if (fract < 0) {
            --num;
            if (num < 0) {
                num += limit;
            }
            fract += 256;
        }
    }
}


gfx::gen::Texture::Texture(RGBAPixmap& pix)
    : m_pixmap(pix)
{ }

void
gfx::gen::Texture::fill(ColorQuad_t color)
{
    m_pixmap.pixels().fill(color);
}

void
gfx::gen::Texture::fillNoise(ColorRange r, util::RandomNumberGenerator& rng)
{
    afl::base::Memory<ColorQuad_t> q(m_pixmap.pixels());
    while (ColorQuad_t* p = q.eat()) {
        *p = r.get(rng(256));
    }
}

void
gfx::gen::Texture::renderCircularGradient(Point center, int radius, ColorRange range, util::RandomNumberGenerator& rng, int noiseScale)
{
    for (int y = 0, height = m_pixmap.getHeight(); y < height; ++y) {
        for (int x = 0, width = m_pixmap.getWidth(); x < width; ++x) {
            double dist = util::getDistanceFromDX(x - center.getX(), y - center.getY());
            if (noiseScale > 0) {
                dist += rng(uint16_t(noiseScale));
            }
            if (dist < radius) {
                if (ColorQuad_t* pix = m_pixmap.row(y).at(x)) {
                    *pix = range.get(int(dist * 255 / radius));
                }
            }
        }
    }
}

void
gfx::gen::Texture::renderBrush(ColorRange r, int count, int angle, util::RandomNumberGenerator& rng)
{
    // Compute direction. Special case for common case to avoid FP trouble.
    int dx, dy;
    angle %= 180;
    if (angle == 0) {
        dx = 256;
        dy = 0;
    } else if (angle == 90) {
        dx = 0;
        dy = 256;
    } else {
        dx = int(256 * std::cos(angle * util::PI / 180));
        dy = int(256 * std::sin(angle * util::PI / 180));
    }

    // Iterate
    int width = m_pixmap.getWidth();
    int height = m_pixmap.getHeight();
    for (int i = 0; i < count; ++i) {
        const ColorQuad_t color = r.get(rng(256));
        int intX = rng(uint16_t(width));
        int intY = rng(uint16_t(height));
        int fractX = 0;
        int fractY = 0;
        int length = rng(uint16_t(width/2)) + 5;
        for (int j = 0; j < length; ++j) {
            uint8_t opacity = uint8_t(OPAQUE_ALPHA * std::sin(util::PI * j / length)); // FIXME: remove trig?
            if (ColorQuad_t* pix = m_pixmap.row(intY).at(intX)) {
                *pix = mixColor(*pix, color, opacity);
            }
            advance(fractX, dx, intX, width);
            advance(fractY, dy, intY, height);
        }
    }
}

gfx::RGBAPixmap&
gfx::gen::Texture::pixmap()
{
    return m_pixmap;
}
