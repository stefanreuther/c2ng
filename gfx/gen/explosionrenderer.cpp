/**
  *  \file gfx/gen/explosionrenderer.cpp
  */

#include "gfx/gen/explosionrenderer.hpp"
#include "gfx/palettizedpixmap.hpp"

namespace {
    // ex makeExplodePixmap
    // These are derived from the default palette
    const gfx::ColorQuad_t EXPLOSION_PALETTE[64] = {
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA( 24,   8,   0, 134),
        COLORQUAD_FROM_RGBA( 48,  16,   0, 136),
        COLORQUAD_FROM_RGBA( 48,  16,   0, 138),
        COLORQUAD_FROM_RGBA( 72,  24,   0, 140),
        COLORQUAD_FROM_RGBA( 72,  24,   0, 142),
        COLORQUAD_FROM_RGBA( 97,  32,   0, 144),
        COLORQUAD_FROM_RGBA( 97,  32,   0, 146),
        COLORQUAD_FROM_RGBA(121,  40,   0, 148),
        COLORQUAD_FROM_RGBA(121,  40,   0, 150),
        COLORQUAD_FROM_RGBA(145,  48,   0, 152),
        COLORQUAD_FROM_RGBA(145,  48,   0, 154),
        COLORQUAD_FROM_RGBA(170,  56,   0, 156),
        COLORQUAD_FROM_RGBA(170,  56,   0, 158),
        COLORQUAD_FROM_RGBA(194,  64,   0, 160),
        COLORQUAD_FROM_RGBA(194,  64,   0, 162),
        COLORQUAD_FROM_RGBA(218,  72,   0, 164),
        COLORQUAD_FROM_RGBA(218,  72,   0, 166),
        COLORQUAD_FROM_RGBA(242,  80,   0, 168),
        COLORQUAD_FROM_RGBA(242,  80,   0, 170),
        COLORQUAD_FROM_RGBA(255,  85,   0, 172),
        COLORQUAD_FROM_RGBA(255,  85,   0, 174),
        COLORQUAD_FROM_RGBA(255,  97,   0, 176),
        COLORQUAD_FROM_RGBA(255,  97,   0, 178),
        COLORQUAD_FROM_RGBA(255, 109,   0, 180),
        COLORQUAD_FROM_RGBA(255, 109,   0, 182),
        COLORQUAD_FROM_RGBA(255, 121,   0, 184),
        COLORQUAD_FROM_RGBA(255, 121,   0, 186),
        COLORQUAD_FROM_RGBA(255, 133,   0, 188),
        COLORQUAD_FROM_RGBA(255, 133,   0, 190),
        COLORQUAD_FROM_RGBA(255, 145,   0, 192),
        COLORQUAD_FROM_RGBA(255, 145,   0, 194),
        COLORQUAD_FROM_RGBA(255, 157,   0, 196),
        COLORQUAD_FROM_RGBA(255, 157,   0, 198),
        COLORQUAD_FROM_RGBA(255, 170,   0, 200),
        COLORQUAD_FROM_RGBA(255, 170,   0, 202),
        COLORQUAD_FROM_RGBA(255, 182,   0, 204),
        COLORQUAD_FROM_RGBA(255, 182,   0, 206),
        COLORQUAD_FROM_RGBA(255, 194,   0, 208),
        COLORQUAD_FROM_RGBA(255, 194,   0, 210),
        COLORQUAD_FROM_RGBA(255, 206,   0, 212),
        COLORQUAD_FROM_RGBA(255, 206,   0, 214),
        COLORQUAD_FROM_RGBA(255, 218,   0, 216),
        COLORQUAD_FROM_RGBA(255, 218,   0, 218),
        COLORQUAD_FROM_RGBA(255, 230,   0, 220),
        COLORQUAD_FROM_RGBA(255, 230,   0, 222),
        COLORQUAD_FROM_RGBA(255, 242,   0, 224),
        COLORQUAD_FROM_RGBA(255, 242,   0, 226),
        COLORQUAD_FROM_RGBA(255, 255,   0, 228),
        COLORQUAD_FROM_RGBA(255, 255,   0, 230),
        COLORQUAD_FROM_RGBA(255, 255,  32, 232),
        COLORQUAD_FROM_RGBA(255, 255,  32, 234),
        COLORQUAD_FROM_RGBA(255, 255,  72, 236),
        COLORQUAD_FROM_RGBA(255, 255,  72, 238),
        COLORQUAD_FROM_RGBA(255, 255, 113, 240),
        COLORQUAD_FROM_RGBA(255, 255, 113, 242),
        COLORQUAD_FROM_RGBA(255, 255, 153, 244),
        COLORQUAD_FROM_RGBA(255, 255, 153, 246),
        COLORQUAD_FROM_RGBA(255, 255, 194, 248),
        COLORQUAD_FROM_RGBA(255, 255, 194, 250),
        COLORQUAD_FROM_RGBA(255, 255, 234, 252),
        COLORQUAD_FROM_RGBA(255, 255, 234, 255),
    };
}

gfx::gen::ExplosionRenderer::ExplosionRenderer(Point area, int size, int speed, util::RandomNumberGenerator& rng)
    : m_renderer(),
      m_area(area),
      m_speed(speed)
{
    // ex VcrExplSprite::getExplosion (sort-of)
    if (size < 32) {
        m_renderer.addParticles(size, Point(m_area.getX()/2, m_area.getY()/2), Point(16384, 16384), Point(8192, 8192), rng);
    } else {
        m_renderer.addParticles(size, Point(m_area.getX()/2, m_area.getY()/2), Point(32767, 32767), Point(16384, 16384), rng);
    }
}

gfx::gen::ExplosionRenderer::~ExplosionRenderer()
{ }

afl::base::Ref<gfx::Canvas>
gfx::gen::ExplosionRenderer::renderFrame()
{
    afl::base::Ref<PalettizedPixmap> pix(PalettizedPixmap::create(m_area.getX(), m_area.getY()));
    pix->setPalette(0, EXPLOSION_PALETTE);
    m_renderer.advanceTime(m_speed);
    m_renderer.render(*pix);
    return pix->makeCanvas();
}

afl::base::Ref<gfx::Canvas>
gfx::gen::ExplosionRenderer::renderAll()
{
    int numFrames = m_renderer.getNumRemainingFrames(m_speed);

    afl::base::Ref<PalettizedPixmap> result(PalettizedPixmap::create(m_area.getX(), m_area.getY() * numFrames));
    result->setPalette(0, EXPLOSION_PALETTE);

    afl::base::Ref<PalettizedPixmap> frame(PalettizedPixmap::create(m_area.getX(), m_area.getY()));
    for (int i = 0; i < numFrames; ++i) {
        m_renderer.advanceTime(m_speed);
        m_renderer.render(*frame);
        result->pixels().subrange(m_area.getX() * m_area.getY() * i).copyFrom(frame->pixels());
    }
    return result->makeCanvas();
}

bool
gfx::gen::ExplosionRenderer::hasMoreFrames() const
{
    return m_renderer.hasMoreFrames();
}
