/**
  *  \file gfx/gen/planetconfig.cpp
  *  \brief Class gfx::gen::PlanetConfig
  */

#include "gfx/gen/planetconfig.hpp"
#include "gfx/gen/planet.hpp"
#include "afl/base/countof.hpp"

namespace {
    /*
     *  Color schemes.
     *
     *  The planet renderer needs a color gradient from which it chooses the terrain colors.
     *  We always give it an 8-element gradient.
     *
     *  The actual gradient is derived from a predefined list of color schemes,
     *  which are mixed according to the planet temperature.
     */

    typedef gfx::ColorQuad_t ColorScheme_t[8];

    ColorScheme_t COLOR_SCHEMES[] = {
        { COLORQUAD_FROM_RGB(0xFF, 0xFF, 0xFF),     // More or less just ice
          COLORQUAD_FROM_RGB(0xEE, 0xEE, 0xEE),
          COLORQUAD_FROM_RGB(0xDD, 0xDD, 0xDD),
          COLORQUAD_FROM_RGB(0xAA, 0xBB, 0xBB),
          COLORQUAD_FROM_RGB(0xAA, 0xAA, 0xAA),
          COLORQUAD_FROM_RGB(0xBB, 0xBB, 0xBB),
          COLORQUAD_FROM_RGB(0xEE, 0xEE, 0xEE),
          COLORQUAD_FROM_RGB(0xFF, 0xFF, 0xFF), },

        { COLORQUAD_FROM_RGB(0xFF, 0xFF, 0xFF),     // Icy with some water
          COLORQUAD_FROM_RGB(0xDD, 0xDD, 0xDD),
          COLORQUAD_FROM_RGB(0xCC, 0xFF, 0xFF),
          COLORQUAD_FROM_RGB(0x80, 0xFF, 0xFF),
          COLORQUAD_FROM_RGB(0x60, 0xC0, 0xC0),
          COLORQUAD_FROM_RGB(0x40, 0x80, 0x80),
          COLORQUAD_FROM_RGB(0x20, 0x40, 0x40),
          COLORQUAD_FROM_RGB(0x00, 0x00, 0x40), },

        { COLORQUAD_FROM_RGB(0xFF, 0xFF, 0xFF),     // Water
          COLORQUAD_FROM_RGB(0xF5, 0xF4, 0xF2),
          COLORQUAD_FROM_RGB(0xD3, 0xCA, 0xDD),
          COLORQUAD_FROM_RGB(0xEF, 0xEB, 0xEB),
          COLORQUAD_FROM_RGB(0xAC, 0xD0, 0xA5),
          COLORQUAD_FROM_RGB(0x20, 0x40, 0x40),
          COLORQUAD_FROM_RGB(0x10, 0x20, 0x20),
          COLORQUAD_FROM_RGB(0x00, 0x00, 0x40), },

        { COLORQUAD_FROM_RGB(0x84, 0x1C, 0x1C),     // Green land and water
          COLORQUAD_FROM_RGB(0x68, 0x83, 0x00),
          COLORQUAD_FROM_RGB(0x14, 0x55, 0x00),
          COLORQUAD_FROM_RGB(0x00, 0x69, 0x00),
          COLORQUAD_FROM_RGB(0x00, 0x0C, 0x80),
          COLORQUAD_FROM_RGB(0x00, 0x0C, 0x55),
          COLORQUAD_FROM_RGB(0x00, 0x0C, 0x80),
          COLORQUAD_FROM_RGB(0x00, 0x0C, 0x55), },

        { COLORQUAD_FROM_RGB(0xA3, 0xA2, 0xA1),     // Much land, some water
          COLORQUAD_FROM_RGB(0x71, 0x5A, 0x37),
          COLORQUAD_FROM_RGB(0x62, 0x7F, 0x5C),
          COLORQUAD_FROM_RGB(0x72, 0x8A, 0x6E),
          COLORQUAD_FROM_RGB(0x6B, 0x8C, 0xA4),
          COLORQUAD_FROM_RGB(0x4B, 0x72, 0x90),
          COLORQUAD_FROM_RGB(0x0A, 0x15, 0x15),
          COLORQUAD_FROM_RGB(0x00, 0x00, 0x2A), },

        { COLORQUAD_FROM_RGB(0xA3, 0xA2, 0xA1),     // Much land, little water
          COLORQUAD_FROM_RGB(0x71, 0x5A, 0x37),
          COLORQUAD_FROM_RGB(0x8C, 0x86, 0x68),
          COLORQUAD_FROM_RGB(0x9F, 0x9C, 0x80),
          COLORQUAD_FROM_RGB(0x62, 0x7F, 0x5C),
          COLORQUAD_FROM_RGB(0x72, 0x8A, 0x6E),
          COLORQUAD_FROM_RGB(0x6B, 0x8C, 0xA4),
          COLORQUAD_FROM_RGB(0x4B, 0x72, 0x90), },

        { COLORQUAD_FROM_RGB(0xFB, 0xCB, 0x80),     // Yellow/brown desert
          COLORQUAD_FROM_RGB(0xFC, 0xC4, 0x79),
          COLORQUAD_FROM_RGB(0xF3, 0xB8, 0x73),
          COLORQUAD_FROM_RGB(0xE7, 0xAD, 0x6C),
          COLORQUAD_FROM_RGB(0xE0, 0xA4, 0x6E),
          COLORQUAD_FROM_RGB(0xDA, 0x9A, 0x58),
          COLORQUAD_FROM_RGB(0xD5, 0x90, 0x4E),
          COLORQUAD_FROM_RGB(0xD0, 0x84, 0x41), },

        { COLORQUAD_FROM_RGB(0xF7, 0xC7, 0x9D),     // Red desert
          COLORQUAD_FROM_RGB(0xE0, 0xAA, 0x7F),
          COLORQUAD_FROM_RGB(0xDD, 0x81, 0x6B),
          COLORQUAD_FROM_RGB(0xDD, 0x89, 0x75),
          COLORQUAD_FROM_RGB(0xD0, 0x78, 0x6A),
          COLORQUAD_FROM_RGB(0xC8, 0x86, 0x65),
          COLORQUAD_FROM_RGB(0xBF, 0x75, 0x5A),
          COLORQUAD_FROM_RGB(0xB1, 0x5F, 0x55), },
    };
}


// Constructor.
gfx::gen::PlanetConfig::PlanetConfig()
    : m_width(640),
      m_height(480),
      m_planetRelX(50),
      m_planetRelY(50),
      m_planetRelRadius(40),
      m_planetTemperature(50),
      m_sunRelX(100),
      m_sunRelY(100),
      m_sunRelZ(-100)
{ }

// Set image size.
void
gfx::gen::PlanetConfig::setSize(Point pt)
{
    m_width = pt.getX();
    m_height = pt.getY();
}

// Set relative position of planet center.
void
gfx::gen::PlanetConfig::setPlanetPosition(int relX, int relY)
{
    m_planetRelX = relX;
    m_planetRelY = relY;
}

// Set relative planet radius.
void
gfx::gen::PlanetConfig::setPlanetRadius(int relRadius)
{
    m_planetRelRadius = relRadius;
}

// Set planet temperature.
void
gfx::gen::PlanetConfig::setPlanetTemperature(int temp)
{
    m_planetTemperature = temp;
}

// Set sun position.
void
gfx::gen::PlanetConfig::setSunPosition(int relX, int relY, int relZ)
{
    m_sunRelX = relX;
    m_sunRelY = relY;
    m_sunRelZ = relZ;
}

// Render.
afl::base::Ref<gfx::RGBAPixmap>
gfx::gen::PlanetConfig::render(util::RandomNumberGenerator& rng) const
{
    // Create canvas
    afl::base::Ref<RGBAPixmap> result = RGBAPixmap::create(m_width, m_height);

    // Scale coordinates. 100% is the size of the square within our canvas.
    const int scale = std::min(m_width, m_height);
    const Planet::ValueVector_t planetPos(m_width * m_planetRelX / 100, m_height * m_planetRelY / 100, 0);
    const int planetRadius = scale * m_planetRelRadius / 100;
    const int clearness = std::abs(50 - m_planetTemperature) / 5 + 2;
    const Planet::ValueVector_t lightSource(scale * m_sunRelX / 100,
                                            scale * m_sunRelY / 100,
                                            scale * m_sunRelZ / 100);

    // Color scheme
    const size_t NUM_SCHEMES = countof(COLOR_SCHEMES);
#if 1
    ColorScheme_t scheme;
    const int TEMP_MAX = 101;
    const int planetTemp = m_planetTemperature % TEMP_MAX;
    size_t schemeSelect = (NUM_SCHEMES-1) * planetTemp / TEMP_MAX;
    size_t schemeMix   = ((NUM_SCHEMES-1) * planetTemp % TEMP_MAX) * 255 / TEMP_MAX;
    for (size_t i = 0; i < countof(scheme); ++i) {
        scheme[i] = mixColor(COLOR_SCHEMES[schemeSelect][i], COLOR_SCHEMES[schemeSelect+1][i], uint8_t(schemeMix));
    }
#else
    const ColorScheme_t& scheme = COLOR_SCHEMES[m_planetTemperature % NUM_SCHEMES];
#endif

    // Render
    Planet(*result).renderPlanet(planetPos,
                                 planetRadius,
                                 scheme,
                                 clearness,
                                 lightSource,
                                 rng);

    return result;
}
