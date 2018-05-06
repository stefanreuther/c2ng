/**
  *  \file gfx/anim/sprite.cpp
  */

#include "gfx/anim/sprite.hpp"

gfx::anim::Sprite::Sprite()
    : m_extent(),
      m_dirty(),
      m_z(0),
      m_id(0),
      m_markedForDeletion(false)
{
    // ex GfxSprite3D::GfxSprite3D
}

void
gfx::anim::Sprite::setExtent(const Rectangle& extent)
{
    // ex GfxSprite3D::setPos, GfxSprite3D::setPos2D
    if (extent != m_extent) {
        m_dirty.include(m_extent);
        m_extent = extent;
        m_dirty.include(m_extent);
    }
}

const gfx::Rectangle&
gfx::anim::Sprite::getExtent() const
{
    // ex GfxSprite3D::getPos
    return m_extent;
}

void
gfx::anim::Sprite::setCenter(Point pt)
{
    // ex GfxSprite3D::setCenterPos2D, GfxSprite3D::setCenterPos
    Point delta = pt - m_extent.getCenter();
    if (delta != Point(0, 0)) {
        m_dirty.include(m_extent);
        m_extent.moveBy(delta);
        m_dirty.include(m_extent);
    }
}

gfx::Point
gfx::anim::Sprite::getCenter() const
{
    // ex GfxSprite3D::getPosCX, getPosCY
    return m_extent.getCenter();
}

void
gfx::anim::Sprite::setZ(int z)
{
    // ex GfxSprite3D::getPosZ
    if (z != m_z) {
        m_z = z;
        markChanged();
    }
}

int
gfx::anim::Sprite::getZ() const
{
    return m_z;
}

void
gfx::anim::Sprite::setId(int id)
{
    m_id = id;
}

int
gfx::anim::Sprite::getId() const
{
    return m_id;
}

void
gfx::anim::Sprite::markChanged()
{
    m_dirty.include(m_extent);
}

void
gfx::anim::Sprite::markClean()
{
    m_dirty = Rectangle();
}

bool
gfx::anim::Sprite::isChanged() const
{
    return m_dirty.exists();
}

const gfx::Rectangle&
gfx::anim::Sprite::getDirtyRegion() const
{
    return m_dirty;
}

void
gfx::anim::Sprite::markForDeletion()
{
    // ex GfxSprite3D::deleteThis
    m_markedForDeletion = true;
}

bool
gfx::anim::Sprite::isMarkedForDeletion() const
{
    return m_markedForDeletion;
}
