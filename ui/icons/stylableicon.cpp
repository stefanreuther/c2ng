/**
  *  \file ui/icons/stylableicon.cpp
  */

#include "ui/icons/stylableicon.hpp"
#include "util/updater.hpp"
#include "gfx/complex.hpp"

using util::Updater;

ui::icons::StylableIcon::StylableIcon(Icon& content, ColorScheme& colors)
    : m_content(content),
      m_colors(colors),
      m_paddingBefore(),
      m_paddingAfter(),
      m_marginBefore(),
      m_marginAfter(),
      m_hasBackgroundColor(false),
      m_backgroundColor(0),
      m_frameType(NoFrame),
      m_frameWidth(0)
{ }

ui::icons::StylableIcon::~StylableIcon()
{ }

bool
ui::icons::StylableIcon::setPaddingBefore(gfx::Point p)
{
    return Updater().set(m_paddingBefore, p);
}

bool
ui::icons::StylableIcon::setPaddingAfter(gfx::Point p)
{
    return Updater().set(m_paddingAfter, p);
}

bool
ui::icons::StylableIcon::setMarginBefore(gfx::Point p)
{
    return Updater().set(m_marginBefore, p);
}

bool
ui::icons::StylableIcon::setMarginAfter(gfx::Point p)
{
    return Updater().set(m_marginAfter, p);
}

bool
ui::icons::StylableIcon::setBackgroundColor(gfx::Color_t color)
{
    return Updater().set(m_hasBackgroundColor, true).set(m_backgroundColor, color);
}

bool
ui::icons::StylableIcon::setFrameWidth(int width)
{
    return Updater().set(m_frameWidth, width);
}

bool
ui::icons::StylableIcon::setFrameType(FrameType type)
{
    return Updater().set(m_frameType, type);
}

gfx::Point
ui::icons::StylableIcon::getSize() const
{
    return m_content.getSize()
        + m_paddingBefore + m_paddingAfter
        + m_marginBefore + m_marginAfter
        + gfx::Point(2*m_frameWidth, 2*m_frameWidth);
}

void
ui::icons::StylableIcon::draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    // Margin
    area.consumeX(m_marginBefore.getX());
    area.consumeY(m_marginBefore.getY());
    area.consumeRightX(m_marginAfter.getX());
    area.consumeBottomY(m_marginAfter.getY());

    // Frame
    if (m_frameWidth != 0) {
        gfx::Context<uint8_t> cctx(ctx.canvas(), m_colors);
        drawFrame(cctx, area, m_frameType, m_frameWidth);
        area.grow(-m_frameWidth, -m_frameWidth);
    }

    // Background
    if (m_hasBackgroundColor) {
        ctx.setRawColor(m_backgroundColor);
        ctx.setFillPattern(gfx::FillPattern::SOLID);
        gfx::drawBar(ctx, area);
    }

    // Padding
    area.consumeX(m_paddingBefore.getX());
    area.consumeY(m_paddingBefore.getY());
    area.consumeRightX(m_paddingAfter.getX());
    area.consumeBottomY(m_paddingAfter.getY());

    // Content
    m_content.draw(ctx, area, flags);
}
