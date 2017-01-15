/**
  *  \file ui/invisiblewidget.cpp
  *  \brief Class ui::InvisibleWidget
  */

#include "ui/invisiblewidget.hpp"

void
ui::InvisibleWidget::draw(gfx::Canvas& /*can*/)
{ }

void
ui::InvisibleWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::InvisibleWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::InvisibleWidget::getLayoutInfo() const
{
    return ui::layout::Info();
}

bool
ui::InvisibleWidget::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
