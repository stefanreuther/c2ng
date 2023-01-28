/**
  *  \file ui/simplewidget.cpp
  */

#include "ui/simplewidget.hpp"

ui::SimpleWidget::SimpleWidget()
    : Widget()
{ }

ui::SimpleWidget::~SimpleWidget()
{ }

void
ui::SimpleWidget::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& /*area*/)
{
    // I don't have any children.
}

void
ui::SimpleWidget::handleChildAdded(Widget& /*child*/)
{
    // I don't have any children.
}

void
ui::SimpleWidget::handleChildRemove(Widget& /*child*/)
{
    // I don't have any children.
}

void
ui::SimpleWidget::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{
    // I don't have any children.
}
