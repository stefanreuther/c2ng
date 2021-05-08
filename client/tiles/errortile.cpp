/**
  *  \file client/tiles/errortile.cpp
  */

#include "client/tiles/errortile.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"

client::tiles::ErrorTile::ErrorTile(String_t text, ui::Root& root)
    : m_text(text),
      m_root(root)
{ }

client::tiles::ErrorTile::~ErrorTile()
{ }

void
client::tiles::ErrorTile::draw(gfx::Canvas& can)
{
    // ex WErrorTile::drawData
    gfx::Rectangle in = getExtent();

    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest()));
    ctx.setColor(ui::Color_Red);
    drawRectangle(ctx, in);

    // Text
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    outTextF(ctx, in, m_text);
}

void
client::tiles::ErrorTile::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::tiles::ErrorTile::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::tiles::ErrorTile::getLayoutInfo() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(20, 2);
}

bool
client::tiles::ErrorTile::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
client::tiles::ErrorTile::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
