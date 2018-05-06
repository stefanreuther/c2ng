/**
  *  \file client/vcr/beamsprite.cpp
  */

#include "client/vcr/beamsprite.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"

client::vcr::BeamSprite::BeamSprite(ui::ColorScheme& cs, gfx::Point a, gfx::Point b)
    : Sprite(),
      m_state(0),
      m_colors(cs),
      m_a(a),
      m_b(b)
{
    // ex VcrBeamSprite::VcrBeamSprite
    setExtent(gfx::Rectangle(std::min(a.getX(), b.getX()),
                             std::min(a.getY(), b.getY()),
                             std::abs(a.getX() - b.getX()) + 1,
                             std::abs(a.getY() - b.getY()) + 1));
}

client::vcr::BeamSprite::~BeamSprite()
{ }

void
client::vcr::BeamSprite::draw(gfx::Canvas& can)
{
    // ex VcrBeamSprite::draw
    static const uint8_t COLORS[LIMIT][3] = {
        { ui::Color_White, ui::Color_DarkCyan, ui::Color_DarkCyan },
        { ui::Color_White, ui::Color_White, ui::Color_DarkCyan },
        { ui::Color_White, ui::Color_White, ui::Color_White },
        { ui::Color_Blue, ui::Color_White, ui::Color_White },
        { ui::Color_Blue, ui::Color_Blue, ui::Color_White },
        { ui::Color_Blue, ui::Color_Blue, ui::Color_Blue },
    };

    int x0 = m_a.getX();
    int y0 = m_a.getY();
    int x1 = m_b.getX();
    int y1 = m_b.getY();
    int x2 = (2*x0+x1)/3;
    int y2 = (2*y0+y1)/3;
    int x3 = (2*x1+x0)/3;
    int y3 = (2*y1+y0)/3;

    gfx::Context<uint8_t> ctx(can, m_colors);
    int index = std::min(std::max(0, m_state-1), LIMIT-1);
    ctx.setColor(COLORS[index][0]);
    drawLine(ctx, gfx::Point(x0, y0), gfx::Point(x2, y2));
    ctx.setColor(COLORS[index][1]);
    drawLine(ctx, gfx::Point(x2, y2), gfx::Point(x3, y3));
    ctx.setColor(COLORS[index][2]);
    drawLine(ctx, gfx::Point(x3, y3), gfx::Point(x1, y1));
}

void
client::vcr::BeamSprite::tick()
{
    // ex VcrBeamSprite::tick
    if (++m_state > LIMIT) {
        markForDeletion();
    } else {
        markChanged();
    }
}
