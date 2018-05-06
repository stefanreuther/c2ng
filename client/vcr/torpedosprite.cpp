/**
  *  \file client/vcr/torpedosprite.cpp
  */

#include "client/vcr/torpedosprite.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

/*
 *  As of 20180318, this is different from PCC2's VcrTorpSprite.
 *  This one: red cross (a la FLAK), constant speed on straight line
 *  PCC2: tiny rocket sprite, accelerates on both axes
 */

client::vcr::TorpedoSprite::TorpedoSprite(ui::ColorScheme& cs, gfx::Point a, gfx::Point b, int time)
    : m_colors(cs),
      m_a(a),
      m_b(b),
      m_time(std::max(time, 1)),
      m_state(0)
{
    // ex VcrTorpSprite::VcrTorpSprite (partial)
    setExtent(gfx::Rectangle(a.getX() - 3, a.getY() - 3, 7, 7));
}

client::vcr::TorpedoSprite::~TorpedoSprite()
{ }

void
client::vcr::TorpedoSprite::draw(gfx::Canvas& can)
{
    gfx::Context<uint8_t> ctx(can, m_colors);
    ctx.setColor(ui::Color_Red);
    drawVLine(ctx, getCenter().getX(), getExtent().getTopY(), getExtent().getBottomY()-1);
    drawHLine(ctx, getExtent().getLeftX(), getCenter().getY(), getExtent().getRightX()-1);
}

void
client::vcr::TorpedoSprite::tick()
{
    if (++m_state > m_time) {
        markForDeletion();
    } else {
        int as = m_time - m_state;
        int bs = m_state;
        int total = m_time;
        setCenter(gfx::Point((m_a.getX() * as + m_b.getX() * bs) / total,
                             (m_a.getY() * as + m_b.getY() * bs) / total));
    }
}
