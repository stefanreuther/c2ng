/**
  *  \file gfx/gen/shieldrenderer.cpp
  */

#include "gfx/gen/shieldrenderer.hpp"

namespace {
    const int SPEED = 2;

    const gfx::ColorQuad_t SHIELD_COLORS[64] = {
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
        COLORQUAD_FROM_RGBA(  0,   0,   0,   6),    // 64
        COLORQUAD_FROM_RGBA(  0,   0,  28,   8),    // 65
        COLORQUAD_FROM_RGBA(  0,   0,  28,  10),    // 65
        COLORQUAD_FROM_RGBA(  0,   0,  28,  12),    // 65
        COLORQUAD_FROM_RGBA(  0,   0,  28,  14),    // 65
        COLORQUAD_FROM_RGBA(  0,   0,  56,  16),    // 66
        COLORQUAD_FROM_RGBA(  0,   0,  56,  18),    // 66
        COLORQUAD_FROM_RGBA(  0,   0,  56,  20),    // 66
        COLORQUAD_FROM_RGBA(  0,   0,  56,  22),    // 66
        COLORQUAD_FROM_RGBA(  0,   0,  85,  24),    // 67
        COLORQUAD_FROM_RGBA(  0,   0,  85,  26),    // 67
        COLORQUAD_FROM_RGBA(  0,   0,  85,  28),    // 67
        COLORQUAD_FROM_RGBA(  0,   0,  85,  30),    // 67
        COLORQUAD_FROM_RGBA( 32,  32, 121,  32),    // 68
        COLORQUAD_FROM_RGBA( 32,  32, 121,  34),    // 68
        COLORQUAD_FROM_RGBA( 32,  32, 121,  36),    // 68
        COLORQUAD_FROM_RGBA( 32,  32, 121,  38),    // 68
        COLORQUAD_FROM_RGBA( 52,  52, 133,  40),    // 69
        COLORQUAD_FROM_RGBA( 52,  52, 133,  42),    // 69
        COLORQUAD_FROM_RGBA( 52,  52, 133,  44),    // 69
        COLORQUAD_FROM_RGBA( 52,  52, 133,  46),    // 69
        COLORQUAD_FROM_RGBA( 72,  72, 145,  48),    // 70
        COLORQUAD_FROM_RGBA( 72,  72, 145,  50),    // 70
        COLORQUAD_FROM_RGBA( 72,  72, 145,  52),    // 70
        COLORQUAD_FROM_RGBA( 72,  72, 145,  54),    // 70
        COLORQUAD_FROM_RGBA( 93,  93, 157,  56),    // 71
        COLORQUAD_FROM_RGBA( 93,  93, 157,  58),    // 71
        COLORQUAD_FROM_RGBA( 93,  93, 157,  60),    // 71
        COLORQUAD_FROM_RGBA( 93,  93, 157,  62),    // 71
        COLORQUAD_FROM_RGBA(113, 113, 170,  64),    // 72
        COLORQUAD_FROM_RGBA(113, 113, 170,  66),    // 72
        COLORQUAD_FROM_RGBA(113, 113, 170,  68),    // 72
        COLORQUAD_FROM_RGBA(113, 113, 170,  70),    // 72
        COLORQUAD_FROM_RGBA(133, 133, 182,  72),    // 73
        COLORQUAD_FROM_RGBA(133, 133, 182,  74),    // 73
        COLORQUAD_FROM_RGBA(133, 133, 182,  76),    // 73
        COLORQUAD_FROM_RGBA(133, 133, 182,  78),    // 73
        COLORQUAD_FROM_RGBA(153, 153, 194,  80),    // 74
        COLORQUAD_FROM_RGBA(153, 153, 194,  82),    // 74
        COLORQUAD_FROM_RGBA(153, 153, 194,  84),    // 74
        COLORQUAD_FROM_RGBA(153, 153, 194,  86),    // 74
        COLORQUAD_FROM_RGBA(174, 174, 206,  88),    // 75
        COLORQUAD_FROM_RGBA(174, 174, 206,  90),    // 75
        COLORQUAD_FROM_RGBA(174, 174, 206,  92),    // 75
        COLORQUAD_FROM_RGBA(174, 174, 206,  94),    // 75
        COLORQUAD_FROM_RGBA(194, 194, 218,  96),    // 76
        COLORQUAD_FROM_RGBA(194, 194, 218,  98),    // 76
        COLORQUAD_FROM_RGBA(194, 194, 218, 100),    // 76
        COLORQUAD_FROM_RGBA(194, 194, 218, 102),    // 76
        COLORQUAD_FROM_RGBA(214, 214, 230, 104),    // 77
        COLORQUAD_FROM_RGBA(214, 214, 230, 106),    // 77
        COLORQUAD_FROM_RGBA(214, 214, 230, 108),    // 77
        COLORQUAD_FROM_RGBA(214, 214, 230, 110),    // 77
        COLORQUAD_FROM_RGBA(234, 234, 242, 112),    // 78
        COLORQUAD_FROM_RGBA(234, 234, 242, 114),    // 78
        COLORQUAD_FROM_RGBA(234, 234, 242, 116),    // 78
        COLORQUAD_FROM_RGBA(234, 234, 242, 118),    // 78
        COLORQUAD_FROM_RGBA(255, 255, 255, 120),    // 79
        COLORQUAD_FROM_RGBA(255, 255, 255, 122),    // 79
        COLORQUAD_FROM_RGBA(255, 255, 255, 124),    // 79
        COLORQUAD_FROM_RGBA(  0,   0,   0,   0),
    };

    struct Parameters {
        int16_t xr, yr;
        int16_t xd, yd;
    };
    const Parameters PARAMETERS[8] = {
        { 32767, 16384, 16384,     0 },         // north
        { 16384, 16384, 16384,     0 },         // northeast
        { 16384, 32767, 16384, 16384 },         // east
        { 16384, 16384, 16384, 16384 },         // southeast
        { 32767, 16384, 16384, 16384 },         // south
        { 16384, 16384,     0, 16384 },         // southwest
        { 16384, 32767,     0, 16384 },         // west
        { 16384, 16384,     0,     0 },         // northwest
    };
}

gfx::gen::ShieldRenderer::ShieldRenderer(Point area, int angle, int size, util::RandomNumberGenerator& rng)
    : m_renderer(),
      m_area(area)
{
    const Parameters& p = PARAMETERS[angle & 7];
    m_renderer.addParticles(size, Point(area.getX()/2, area.getY()/2), Point(p.xr, p.yr), Point(p.xd, p.yd), rng);
}

gfx::gen::ShieldRenderer::~ShieldRenderer()
{ }

afl::base::Ref<gfx::Canvas>
gfx::gen::ShieldRenderer::renderFrame()
{
    afl::base::Ref<PalettizedPixmap> pix(PalettizedPixmap::create(m_area.getX(), m_area.getY()));
    pix->setPalette(0, SHIELD_COLORS);
    m_renderer.advanceTime(SPEED);
    m_renderer.render(*pix);
    return pix->makeCanvas();
}

afl::base::Ref<gfx::Canvas>
gfx::gen::ShieldRenderer::renderAll()
{
    int numFrames = m_renderer.getNumRemainingFrames(SPEED);

    afl::base::Ref<PalettizedPixmap> result(PalettizedPixmap::create(m_area.getX(), m_area.getY() * numFrames));
    result->setPalette(0, SHIELD_COLORS);

    afl::base::Ref<PalettizedPixmap> frame(PalettizedPixmap::create(m_area.getX(), m_area.getY()));
    for (int i = 0; i < numFrames; ++i) {
        m_renderer.advanceTime(SPEED);
        m_renderer.render(*frame);
        result->pixels().subrange(m_area.getX() * m_area.getY() * i).copyFrom(frame->pixels());
    }
    return result->makeCanvas();
}

bool
gfx::gen::ShieldRenderer::hasMoreFrames() const
{
    return m_renderer.hasMoreFrames();
}
