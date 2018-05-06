/**
  *  \file gfx/gen/orbitconfig.cpp
  */

#include <algorithm>
#include "gfx/gen/orbitconfig.hpp"
#include "gfx/gen/spaceview.hpp"
#include "gfx/gen/planet.hpp"

// Constructor.
gfx::gen::OrbitConfig::OrbitConfig()
    : m_width(640),
      m_height(480),
      m_numStars(5),
      m_planetRelX(100),
      m_planetRelY(500),
      m_planetRelRadius(415)
{ }

// Set image size.
void
gfx::gen::OrbitConfig::setSize(Point pt)
{
    m_width = pt.getX();
    m_height = pt.getY();
}

// Set number of stars (far stars).
void
gfx::gen::OrbitConfig::setNumStars(int n)
{
    m_numStars = n;
}

// Set relative position of planet center.
void
gfx::gen::OrbitConfig::setPlanetPosition(int relX, int relY)
{
    m_planetRelX = relX;
    m_planetRelY = relY;
}

// Set relative planet radius.
void
gfx::gen::OrbitConfig::setPlanetRadius(int relRadius)
{
    m_planetRelRadius = relRadius;
}

// Render.
afl::base::Ref<gfx::RGBAPixmap>
gfx::gen::OrbitConfig::render(util::RandomNumberGenerator& rng) const
{
    afl::base::Ref<RGBAPixmap> pix = RGBAPixmap::create(m_width, m_height);
    int scale = std::max(m_width, m_height);

    // Starfield
    SpaceView sv(*pix);
    sv.renderStarfield(rng);

    // Stars
    for (int i = 0; i < m_numStars; ++i) {
        int x = rng(uint16_t(m_width));
        int y = rng(uint16_t(m_height));
        SpaceView::Value_t size = rng(uint16_t(scale)) * 0.001;
        sv.renderStar(COLORQUAD_FROM_RGBA(255, 255, 255, 0), Point(x, y), size);
    }

    // One nebula
    uint8_t r = uint8_t(rng(128));
    uint8_t g = uint8_t(rng(128));
    uint8_t b = uint8_t(rng(128));
    SpaceView::Value_t intensity = (rng(256) + 1280) * (1.0 / 1280);      // [1, 1.2)
    SpaceView::Value_t falloff = (rng(768) + 768) * (1.0 / 256);          // [3, 6)
    sv.renderNebula(rng, COLORQUAD_FROM_RGBA(r, g, b, 0), scale / 4, intensity, falloff);

    // Planet
    const ColorQuad_t COLORS[] = {
        COLORQUAD_FROM_RGB(0xFF, 0xFF, 0xFF),
        COLORQUAD_FROM_RGB(2*r,  2*g,  2*b),
        COLORQUAD_FROM_RGB(2*r,  2*g,  2*b),
        COLORQUAD_FROM_RGB(r,    g,    b),
        COLORQUAD_FROM_RGB(r/2,  g/2,  b),
        COLORQUAD_FROM_RGB(r/2,  g,    b/2),
    };

    Planet(*pix).renderPlanet(Planet::ValueVector_t(m_width * m_planetRelX / 100, m_height * m_planetRelY / 100, 0),
                              std::min(m_width, m_height)*m_planetRelRadius/100,
                              COLORS,
                              3,
                              Planet::ValueVector_t(0, 0, -10000),
                              rng);

    // Everything is opaque
    pix->setAlpha(OPAQUE_ALPHA);
    return pix;
}
