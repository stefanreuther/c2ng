/**
  *  \file ui/group.cpp
  */

#include "ui/group.hpp"

ui::Group::Group(const ui::layout::Manager& mgr) throw()
    : LayoutableGroup(mgr)
{ }

gfx::Rectangle
ui::Group::transformSize(gfx::Rectangle size, Transformation /*kind*/) const
{
    return size;
}

void
ui::Group::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
ui::Group::handleStateChange(State /*st*/, bool /*enable*/)
{ }

bool
ui::Group::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::Group::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
