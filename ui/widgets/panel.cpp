/**
  *  \file ui/widgets/panel.cpp
  */

#include "ui/widgets/panel.hpp"

ui::widgets::Panel::Panel(ui::layout::Manager& mgr, int padding)
    : LayoutableGroup(mgr),
      m_padding(padding)
{ }

void
ui::widgets::Panel::setPadding(int padding)
{
    m_padding = padding;
}

gfx::Rectangle
ui::widgets::Panel::transformSize(gfx::Rectangle size, Transformation kind) const
{
    switch (kind) {
     case InnerToOuter:
        size.grow(m_padding, m_padding);
        break;

     case OuterToInner:
        size.grow(-m_padding, -m_padding);
        break;
    }
    return size;
}

void
ui::widgets::Panel::draw(gfx::Canvas& can)
{
    getColorScheme().drawBackground(can, getExtent());
    defaultDrawChildren(can);
}

bool
ui::widgets::Panel::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::Panel::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
