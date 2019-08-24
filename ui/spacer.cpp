/**
  *  \file ui/spacer.cpp
  *  \brief Class ui::Spacer
  */

#include "ui/spacer.hpp"

// Construct growable spacer.
ui::Spacer::Spacer() throw()
    : SimpleWidget(),
      m_info(gfx::Point(), gfx::Point(), ui::layout::Info::GrowBoth)
{ }

// Construct fixed-size spacer.
ui::Spacer::Spacer(gfx::Point size) throw()
    : SimpleWidget(),
      m_info(size)
{ }

// Construct custom spacer.
ui::Spacer::Spacer(ui::layout::Info info) throw()
    : SimpleWidget(),
      m_info(info)
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
    // ex UISpacer::getLayoutInfo, UIFixedSpacer::getLayoutInfo
    return m_info;
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
