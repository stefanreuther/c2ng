/**
  *  \file ui/invisiblewidget.cpp
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
