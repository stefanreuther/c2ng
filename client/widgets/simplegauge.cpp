/**
  *  \file client/widgets/simplegauge.cpp
  */

#include "client/widgets/simplegauge.hpp"
#include "gfx/complex.hpp"
#include "ui/colorscheme.hpp"
#include "ui/draw.hpp"
#include "util/updater.hpp"

client::widgets::SimpleGauge::SimpleGauge(ui::Root& root, int width)
    : m_root(root),
      m_have(0),
      m_total(0),
      m_width(width),
      m_text(),
      m_barColor(ui::Color_Green),
      m_textColor(ui::Color_White)
{ }

client::widgets::SimpleGauge::~SimpleGauge()
{ }

void
client::widgets::SimpleGauge::setValues(int have, int total, String_t text)
{
    if (util::Updater().set(m_have, have).set(m_total, total).set(m_text, text)) {
        requestRedraw();
    }
}

void
client::widgets::SimpleGauge::setBarColor(uint8_t color)
{
    if (color != m_barColor) {
        m_barColor = color;
        requestRedraw();
    }
}

void
client::widgets::SimpleGauge::setTextColor(uint8_t color)
{
    if (color != m_textColor) {
        m_textColor = color;
        requestRedraw();
    }
}

void
client::widgets::SimpleGauge::draw(gfx::Canvas& can)
{
    // ex WShipCargoTile::drawCargoBar
    gfx::Context<uint8_t> ctx(can, m_root.colorScheme());
    
    gfx::Rectangle area = getExtent();
    ui::drawFrameDown(ctx, area);
    area.grow(-1, -1);

    int width = area.getWidth();
    int split = (m_total > 0
                 ? width * m_have / m_total
                 : 0);

    // Finetune the split:
    if (m_have > 0 && split == 0) {
        // Have nonzero, but rounded to zero: show one
        split = 1;
    }
    if (split > width) {
        // Do not overflow to the right
        split = width;
    }

    // Draw bars
    gfx::Rectangle barArea = area;
    drawSolidBar(ctx, barArea.splitX(split), m_barColor);
    drawSolidBar(ctx, barArea, ui::Color_Black);

    // Draw text
    ctx.useFont(*m_root.provider().getFont(gfx::FontRequest().addSize(-1)));
    ctx.setTextAlign(1, 1);
    ctx.setColor(m_textColor);
    outTextF(ctx, area, m_text);
}

void
client::widgets::SimpleGauge::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::SimpleGauge::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::SimpleGauge::getLayoutInfo() const
{
    int textHeight = m_root.provider().getFont(gfx::FontRequest().addSize(-1))->getTextHeight("Tp");
    gfx::Point size(m_width, textHeight + 2);
    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
client::widgets::SimpleGauge::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
client::widgets::SimpleGauge::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
