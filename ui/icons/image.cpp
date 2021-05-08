/**
  *  \file ui/icons/image.cpp
  */

#include "ui/icons/image.hpp"
#include "gfx/complex.hpp"

ui::icons::Image::Image(gfx::Point size)
    : Icon(),
      m_size(size),
      m_image()
{ }

ui::icons::Image::Image(afl::base::Ref<gfx::Canvas> image)
    : Icon(),
      m_size(image->getSize()),
      m_image(image.asPtr())
{ }

ui::icons::Image::~Image()
{ }

bool
ui::icons::Image::setImage(afl::base::Ptr<gfx::Canvas> image)
{
    if (image.get() != m_image.get()) {
        m_image = image;
        return true;
    } else {
        return false;
    }
}

gfx::Point
ui::icons::Image::getSize() const
{
    return m_size;
}

void
ui::icons::Image::draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t /*flags*/) const
{
    if (m_image.get() != 0) {
        gfx::Rectangle imageArea(gfx::Point(), m_image->getSize());
        imageArea.centerWithin(area);
        blitSized(ctx, imageArea, *m_image);
    }
}
