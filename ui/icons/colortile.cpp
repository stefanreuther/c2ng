/**
  *  \file ui/icons/colortile.cpp
  *  \brief Class ui::icons::ColorTile
  */

#include "ui/icons/colortile.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"
#include "util/updater.hpp"

using util::Updater;

ui::icons::ColorTile::ColorTile(Root& root, gfx::Point size, uint8_t color)
    : Icon(), m_root(root), m_size(size), m_color(color), m_frameWidth(1), m_frameType(RaisedFrame)
{ }

ui::icons::ColorTile::~ColorTile()
{ }

bool
ui::icons::ColorTile::setFrameWidth(int frameWidth)
{
    return Updater().set(m_frameWidth, frameWidth);
}

bool
ui::icons::ColorTile::setColor(uint8_t color)
{
    return Updater().set(m_color, color);
}

bool
ui::icons::ColorTile::setFrameType(FrameType ft)
{
    return Updater().set(m_frameType, ft);
}

gfx::Point
ui::icons::ColorTile::getSize() const
{
    return m_size;
}

void
ui::icons::ColorTile::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t /*flags*/) const
{
    gfx::Context<uint8_t> paletteContext(ctx.canvas(), m_root.colorScheme());
    drawFrame(paletteContext, area, m_frameType, m_frameWidth);
    area.grow(-m_frameWidth, -m_frameWidth);
    drawSolidBar(paletteContext, area, m_color);
}
