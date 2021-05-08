/**
  *  \file ui/icons/hbox.cpp
  *  \brief Class ui::icons::HBox
  */

#include "ui/icons/hbox.hpp"

ui::icons::HBox::HBox()
    : m_children(),
      m_alignX(gfx::LeftAlign),
      m_alignY(gfx::MiddleAlign),
      m_pad(0)
{ }

ui::icons::HBox::~HBox()
{ }

void
ui::icons::HBox::add(Icon& icon)
{
    m_children.push_back(&icon);
}

void
ui::icons::HBox::setAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    m_alignX = x;
    m_alignY = y;
}

void
ui::icons::HBox::setPad(int pad)
{
    m_pad = pad;
}

gfx::Point
ui::icons::HBox::getSize() const
{
    gfx::Point result;
    for (size_t i = 0, n = m_children.size(); i < n; ++i) {
        if (i != 0) {
            result.addX(m_pad);
        }
        result.extendRight(m_children[i]->getSize());
    }
    return result;
}

void
ui::icons::HBox::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    for (size_t i = 0, n = m_children.size(); i < n; ++i) {
        // Determine X position
        gfx::Point childSize = m_children[i]->getSize();
        gfx::Rectangle childArea;
        if (m_alignX == gfx::LeftAlign) {
            childArea = area.splitX(childSize.getX());
            area.consumeX(m_pad);
        } else {
            childArea = area.splitRightX(childSize.getX());
            area.consumeRightX(m_pad);
        }

        // Determine Y position
        childArea.consumeY((childArea.getHeight() - childSize.getY()) * m_alignY / 2);
        childArea.consumeBottomY(childArea.getHeight() - childSize.getY());

        // Draw
        m_children[i]->draw(ctx, childArea, flags);
    }
}
