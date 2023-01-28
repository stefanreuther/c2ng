/**
  *  \file client/widgets/cargotransferheader.cpp
  */

#include "client/widgets/cargotransferheader.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "gfx/font.hpp"
#include "ui/draw.hpp"
#include "ui/layout/info.hpp"

client::widgets::CargoTransferHeader::CargoTransferHeader(ui::Root& root, afl::string::Translator& tx, String_t leftName, String_t rightName)
    : m_root(root),
      m_translator(tx),
      m_leftName(leftName),
      m_rightName(rightName)
      // FIXME: header(requestPixmap("greentile"))
{
    // ex WCargoHeader::WCargoHeader
    setState(DisabledState, true);
}

void
client::widgets::CargoTransferHeader::draw(gfx::Canvas& can)
{
    // ex WCargoHeader::drawContent (sort-of)
    gfx::Rectangle area = getExtent();
    area.grow(-1, 0);

    int panelWidth = area.getWidth() / 3;
    int midWidth = area.getWidth() - 2*panelWidth;

    drawHeader(can, area.splitX(panelWidth), m_leftName);
    area.consumeX(midWidth);
    drawHeader(can, area, m_rightName);
}

void
client::widgets::CargoTransferHeader::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::CargoTransferHeader::handlePositionChange()
{ }

ui::layout::Info
client::widgets::CargoTransferHeader::getLayoutInfo() const
{
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest())->getCellSize().scaledBy(50, 3) + gfx::Point(2, 0);
    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::CargoTransferHeader::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
client::widgets::CargoTransferHeader::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}

void
client::widgets::CargoTransferHeader::drawHeader(gfx::Canvas& can, gfx::Rectangle area, const String_t& name)
{
    // ex WCargoHeader::showTitle, transfer.pas:ShowTitle

    // Frame and content
    afl::base::Ref<gfx::Font> font(m_root.provider().getFont(gfx::FontRequest()));
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    ctx.useFont(*font);
    ui::drawFrameDown(ctx, area);
    area.grow(-1, -1);
    // FIXME -> drawTiledArea(can, toprect, header, standard_colors[COLOR_GREENSCALE+4], 0 /* alteration */);
    drawSolidBar(ctx, area, ui::Color_GreenScale+4);

    // Leave room on sides
    area.grow(-font->getEmWidth()/2, 0);

    // Top half
    gfx::Rectangle top(area.splitY(area.getHeight()/2));
    ctx.setColor(ui::Color_White);
    ctx.setTextAlign(gfx::LeftAlign, gfx::BottomAlign);
    top.grow(0, -font->getLineHeight() / 4);
    outTextF(ctx, top, name);
    drawHLine(ctx, top.getLeftX(), top.getBottomY(), top.getRightX());

    // Bottom half
    ctx.setTextAlign(gfx::RightAlign, gfx::TopAlign);
    outTextF(ctx, area.splitX(area.getWidth() / 2), m_translator("has"));
    outTextF(ctx, area,                             m_translator("space left"));
}
