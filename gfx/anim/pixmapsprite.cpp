/**
  *  \file gfx/anim/pixmapsprite.cpp
  */

#include "gfx/anim/pixmapsprite.hpp"

gfx::anim::PixmapSprite::PixmapSprite(afl::base::Ptr<Canvas> pix)
    : m_pixmap()
{
    // ex GfxPixmapSprite3D::GfxPixmapSprite3D
    setPixmap(pix);
}

gfx::anim::PixmapSprite::~PixmapSprite()
{ }

void
gfx::anim::PixmapSprite::setPixmap(afl::base::Ptr<Canvas> pix)
{
    Point oldCenter = getCenter();
    m_pixmap = pix;
    if (m_pixmap.get() != 0) {
        Point pixSize = m_pixmap->getSize();
        setExtent(Rectangle(oldCenter.getX() - pixSize.getX()/2,
                            oldCenter.getY() - pixSize.getY()/2,
                            pixSize.getX(),
                            pixSize.getY()));
    }
}

void
gfx::anim::PixmapSprite::draw(Canvas& can)
{
    // ex GfxPixmapSprite3D::draw
    if (m_pixmap.get() != 0) {
        can.blit(getExtent().getTopLeft(), *m_pixmap, Rectangle(Point(), m_pixmap->getSize()));
    }
}

void
gfx::anim::PixmapSprite::tick()
{ }
