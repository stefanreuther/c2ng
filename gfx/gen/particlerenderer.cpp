/**
  *  \file gfx/gen/particlerenderer.cpp
  */

#include "gfx/gen/particlerenderer.hpp"

namespace {
    // ex VcrExplSprite::EXPLODE_FRAMES
    const int MAX_FRAMES = 74;
}


gfx::gen::ParticleRenderer::ParticleRenderer()
    : m_particles(),
      m_time(0)
{ }

gfx::gen::ParticleRenderer::~ParticleRenderer()
{ }

void
gfx::gen::ParticleRenderer::addParticles(size_t count, Point pos, Point fractionalSpeed, Point fractionalSpeedDelta, util::RandomNumberGenerator& rng)
{
    // ex client/vcranim.cc:makeExplosion (part)
    for (size_t i = 0; i < count; ++i) {
        Particle p;
        p.x = pos.getX() * 65536;
        p.y = pos.getY() * 65536;
        p.dx = rng(uint16_t(fractionalSpeed.getX())) - fractionalSpeedDelta.getX();
        p.dy = rng(uint16_t(fractionalSpeed.getY())) - fractionalSpeedDelta.getY();
        m_particles.push_back(p);
    }
}

void
gfx::gen::ParticleRenderer::render(PalettizedPixmap& pix)
{
    // Start empty
    pix.pixels().fill(0);

    // Base color slot:
    //   0 .. 32 in 16 ticks, 32 to 0 in 64 ticks
    int color = (m_time < 16) ? 2*m_time : (80-m_time) / 2;

    // Place all hotspots
    static const uint8_t SHIFT_TAB[] = {
        4, 3, 3, 3, 2, 2, 2, 2, 2, 3, 3, 3, 4,
        3, 3, 3, 2, 2, 2, 1, 2, 2, 2, 3, 3, 3,
        3, 3, 2, 2, 2, 1, 1, 1, 2, 2, 2, 3, 3,
        3, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 3,
        2, 2, 2, 1, 1, 1, 0, 1, 1, 1, 2, 2, 2,
        2, 2, 1, 1, 1, 0, 0, 0, 1, 1, 1, 2, 2,
        2, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 2,
        2, 2, 1, 1, 1, 0, 0, 0, 1, 1, 1, 2, 2,
        2, 2, 2, 1, 1, 1, 0, 1, 1, 1, 2, 2, 2,
        3, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 3,
        3, 3, 2, 2, 2, 1, 1, 1, 2, 2, 2, 3, 3,
        3, 3, 3, 2, 2, 2, 1, 2, 2, 2, 3, 3, 3,
        4, 3, 3, 3, 2, 2, 2, 2, 2, 3, 3, 3, 4
    };
    for (std::vector<Particle>::iterator it = m_particles.begin(), end = m_particles.end(); it != end; ++it) {
        const uint8_t* shift = SHIFT_TAB;
        int px = (it->x >> 16);
        int py = (it->y >> 16);
        for (int y = -6; y <= 6; ++y) {
            afl::base::Memory<uint8_t> row = pix.row(py + y);
            for (int x = -6; x <= 6; ++x) {
                if (uint8_t* p = row.at(px + x)) {
                    *p = static_cast<uint8_t>(*p + (color >> *shift));
                }
                ++shift;
            }
        }
    }

    // Limit to 64 colors
    // This intentionally lets the above code overflow uint8_t range (instead of limiting right at the addition operation)
    // because it looks nicer for explosions.
    afl::base::Memory<uint8_t> pixels = pix.pixels();
    while (uint8_t* p = pixels.eat()) {
        if (*p >= NUM_COLORS) {
            *p = NUM_COLORS-1;
        }
    }
}

void
gfx::gen::ParticleRenderer::advanceTime(int time)
{
    for (std::vector<Particle>::iterator it = m_particles.begin(), end = m_particles.end(); it != end; ++it) {
        it->x += it->dx;
        it->y += it->dy;
    }
    m_time += time;
}

bool
gfx::gen::ParticleRenderer::hasMoreFrames() const
{
    return m_time < MAX_FRAMES;
}

int
gfx::gen::ParticleRenderer::getNumRemainingFrames(int timePerFrame) const
{
    int remainingTime = (MAX_FRAMES - m_time);
    if (timePerFrame > 0) {
        remainingTime /= timePerFrame;
    }
    return remainingTime;
}
