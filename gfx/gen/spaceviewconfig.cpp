/**
  *  \file gfx/gen/spaceviewconfig.cpp
  *  \brief Class gfx::gen::SpaceViewConfig
  */

#include <algorithm>
#include "gfx/gen/spaceviewconfig.hpp"
#include "gfx/gen/spaceview.hpp"

gfx::gen::SpaceViewConfig::SpaceViewConfig()
    : m_width(640),
      m_height(480),
      m_numSuns(1),
      m_starProbability(95)
{ }

void
gfx::gen::SpaceViewConfig::setSize(Point pt)
{
    m_width = pt.getX();
    m_height = pt.getY();
}

void
gfx::gen::SpaceViewConfig::setNumSuns(int n)
{
    m_numSuns = n;
}

void
gfx::gen::SpaceViewConfig::setStarProbability(int n)
{
    m_starProbability = n;
}

afl::base::Ref<gfx::RGBAPixmap>
gfx::gen::SpaceViewConfig::render(util::RandomNumberGenerator& rng) const
{
    typedef SpaceView::Value_t Value_t;

    // Create canvas
    afl::base::Ref<RGBAPixmap> result = RGBAPixmap::create(m_width, m_height);
    SpaceView renderer(*result);

    // Scale factor to scale things
    const int scale = std::max(m_width, m_height);

    // Render Starfield (very far stars).
    // Since the number of stars may vary depending on the size,
    // use a copy of the RNG so that following steps keep seeing the same state.
    {
        util::RandomNumberGenerator starRNG(rng);
        renderer.renderStarfield(starRNG);
    }

    // Render stars (not so far stars).
    if (m_starProbability > 0) {
        do {
            int x = rng(m_width);
            int y = rng(m_height);
            Value_t size = rng(scale) * 0.001;
            renderer.renderStar(COLORQUAD_FROM_RGBA(255, 255, 255, 0), Point(x, y), size);
        } while (rng(100) < m_starProbability);
    }

    // Render nebulas.
    do {
        uint8_t r = rng(256);
        uint8_t g = rng(256);
        uint8_t b = rng(256);
        Value_t intensity = (rng(256) + 1280) * (1.0 / 1280);      // [1, 1.2)
        Value_t falloff = (rng(768) + 768) * (1.0 / 256);          // [3, 6)
        renderer.renderNebula(rng, COLORQUAD_FROM_RGBA(r, g, b, 0), scale / 4, intensity, falloff);
    } while (rng(2) < 1);

    // Render suns (close stars).
    for (int i = 0; i < m_numSuns; ++i) {
        ColorQuad_t color;
        if (rng(2) == 0) {
            // Colder sun
            uint8_t r = 255;
            uint8_t g = rng(256);
            uint8_t b = rng(64);
            color = COLORQUAD_FROM_RGBA(r, g, b, 0);
        } else {
            // Hotter sun
            uint8_t r = rng(64);
            uint8_t g = rng(256);
            uint8_t b = 255;
            color = COLORQUAD_FROM_RGBA(r, g, b, 0);
        }

        int x = rng(m_width);
        int y = rng(m_height);
        int size = rng(scale / 10) + scale / 100;
        renderer.renderSun(color, Point(x, y), size);
    }

    // The resulting image will have varying alpha values. Set them all to opaque.
    result->setAlpha(OPAQUE_ALPHA);

    return result;
}
