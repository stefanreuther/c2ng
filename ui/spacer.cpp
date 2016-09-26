/**
  *  \file ui/spacer.cpp
  */

#include "ui/spacer.hpp"

// /** \class UISpacer

//     An invisible spacer. When put into a group with other widgets,
//     spacers will expand to fill available space. Spacers themselves
//     are invisible. */
ui::Spacer::Spacer()
{ }

ui::Spacer::~Spacer()
{ }

void
ui::Spacer::draw(gfx::Canvas& /*can*/)
{ }

void
ui::Spacer::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::Spacer::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::Spacer::getLayoutInfo() const
{
    return ui::layout::Info(gfx::Point(), gfx::Point(), ui::layout::Info::GrowBoth);
}

bool
ui::Spacer::handleKey(util::Key_t /*key*/, int /*prefix*/)
{
    return false;
}

bool
ui::Spacer::handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
{
    return false;
}
