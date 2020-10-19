/**
  *  \file ui/rich/stylableobject.cpp
  */

#include "ui/rich/stylableobject.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"

ui::rich::StylableObject::StylableObject(std::auto_ptr<BlockObject> content, ColorScheme& colors)
    : m_content(content),
      m_colors(colors),
      m_paddingBefore(),
      m_paddingAfter(),
      m_marginBefore(),
      m_marginAfter(),
      m_backgroundColor(),
      m_frameType(NoFrame),
      m_frameWidth(0)
{ }

ui::rich::StylableObject::~StylableObject()
{ }

void
ui::rich::StylableObject::setPaddingBefore(gfx::Point p)
{
    m_paddingBefore = p;
}

void
ui::rich::StylableObject::setPaddingAfter(gfx::Point p)
{
    m_paddingAfter = p;
}

void
ui::rich::StylableObject::setMarginBefore(gfx::Point p)
{
    m_marginBefore = p;
}

void
ui::rich::StylableObject::setMarginAfter(gfx::Point p)
{
    m_marginAfter = p;
}

void
ui::rich::StylableObject::setBackgroundColor(gfx::Color_t color)
{
    m_backgroundColor = color;
}

void
ui::rich::StylableObject::setFrameWidth(int width)
{
    m_frameWidth = width;
}
void
ui::rich::StylableObject::setFrameType(FrameType type)
{
    m_frameType = type;
}

gfx::Point
ui::rich::StylableObject::getSize()
{
    return m_content->getSize()
        + m_paddingBefore + m_paddingAfter
        + m_marginBefore + m_marginAfter
        + gfx::Point(2*m_frameWidth, 2*m_frameWidth);
}

void
ui::rich::StylableObject::draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area)
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
    gfx::Color_t color;
    if (m_backgroundColor.get(color)) {
        ctx.setRawColor(color);
        ctx.setFillPattern(gfx::FillPattern::SOLID);
        gfx::drawBar(ctx, area);
    }

    // Padding
    area.consumeX(m_paddingBefore.getX());
    area.consumeY(m_paddingBefore.getY());
    area.consumeRightX(m_paddingAfter.getX());
    area.consumeBottomY(m_paddingAfter.getY());

    // Content
    m_content->draw(ctx, area);
}
