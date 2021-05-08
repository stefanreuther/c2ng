/**
  *  \file ui/icons/spacer.cpp
  *  \brief Class ui::icons::Spacer
  */

#include "ui/icons/spacer.hpp"

ui::icons::Spacer::Spacer(gfx::Point size)
    : Icon(),
      m_size(size)
{ }

ui::icons::Spacer::~Spacer()
{ }

gfx::Point
ui::icons::Spacer::getSize() const
{
    return m_size;
}

void
ui::icons::Spacer::draw(gfx::Context<SkinColor::Color>& /*ctx*/, gfx::Rectangle /*area*/, ButtonFlags_t /*flags*/) const
{ }
