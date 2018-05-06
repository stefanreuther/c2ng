/**
  *  \file ui/widgets/spritewidget.cpp
  */

#include "ui/widgets/spritewidget.hpp"
#include "gfx/clipfilter.hpp"

ui::widgets::SpriteWidget::SpriteWidget()
    : m_controller()
{ }

ui::widgets::SpriteWidget::~SpriteWidget()
{ }

gfx::anim::Controller&
ui::widgets::SpriteWidget::controller()
{
    return m_controller;
}

void
ui::widgets::SpriteWidget::tick()
{
    m_controller.tick();
    requestRedraw(m_controller.getDirtyRegion());
}

void
ui::widgets::SpriteWidget::draw(gfx::Canvas& can)
{
    getColorScheme().drawBackground(can, getExtent());

    gfx::ClipFilter filter(can, getExtent());
    m_controller.draw(filter);
}

void
ui::widgets::SpriteWidget::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::SpriteWidget::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::SpriteWidget::getLayoutInfo() const
{
    return ui::layout::Info();
}

bool
ui::widgets::SpriteWidget::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::widgets::SpriteWidget::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
