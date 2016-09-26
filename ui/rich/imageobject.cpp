/**
  *  \file ui/rich/imageobject.cpp
  */

#include "ui/rich/imageobject.hpp"
#include "gfx/complex.hpp"
#include "util/skincolor.hpp"

ui::rich::ImageObject::ImageObject(afl::base::Ptr<gfx::Canvas> image)
    : m_image(image)
{
    // ex RichDocumentImageObject::RichDocumentImageObject
}

ui::rich::ImageObject::~ImageObject()
{ }

gfx::Point
ui::rich::ImageObject::getSize()
{
    // ex RichDocumentImageObject::getSize
    if (m_image.get() != 0) {
        return m_image->getSize();
    } else {
        return gfx::Point(10, 10);
    }
}

void
ui::rich::ImageObject::draw(gfx::Context& ctx, gfx::Rectangle area)
{
    // ex RichDocumentImageObject::draw
    if (m_image.get() != 0) {
        blitSized(ctx, area, *m_image);
    } else {
        // No image given. This is an error.
        ctx.setColor(util::SkinColor::Red);
        drawRectangle(ctx, area);
    }
}
