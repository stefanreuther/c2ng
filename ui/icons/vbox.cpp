/**
  *  \file ui/icons/vbox.cpp
  */

#include "ui/icons/vbox.hpp"

ui::icons::VBox::VBox()
    : m_children(),
      m_align(gfx::LeftAlign),
      m_pad(0)
{ }

ui::icons::VBox::~VBox()
{ }

void
ui::icons::VBox::add(Icon& icon)
{
    m_children.push_back(&icon);
}

void
ui::icons::VBox::setAlign(gfx::HorizontalAlignment align)
{
    m_align = align;
}

void
ui::icons::VBox::setPad(int pad)
{
    m_pad = pad;
}

gfx::Point
ui::icons::VBox::getSize() const
{
    gfx::Point result;
    for (size_t i = 0, n = m_children.size(); i < n; ++i) {
        if (i != 0) {
            result.addY(m_pad);
        }
        result.extendBelow(m_children[i]->getSize());
    }
    return result;
}

void
ui::icons::VBox::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const
{
    for (size_t i = 0, n = m_children.size(); i < n; ++i) {
        // Determine Y position
        gfx::Point childSize = m_children[i]->getSize();
        gfx::Rectangle childArea = area.splitY(childSize.getY());
        area.consumeY(m_pad);

        // Determine X position
        childArea.consumeX((childArea.getWidth() - childSize.getX()) * m_align / 2);
        childArea.consumeRightX(childArea.getWidth() - childSize.getX());

        // Draw
        m_children[i]->draw(ctx, childArea, flags);
    }
}
