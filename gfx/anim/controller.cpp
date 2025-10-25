/**
  *  \file gfx/anim/controller.cpp
  */

#include "gfx/anim/controller.hpp"

namespace {
    struct CompareSprites {
        bool operator()(const gfx::anim::Sprite& a, const gfx::anim::Sprite& b) const
            {
                // ex gfx/sprite3d.cc:sortByDepth
                // FIXME: we may need to have to do something to get the sort stable.
                return a.getZ() < b.getZ();
            }
    };
}


gfx::anim::Controller::Controller()
    : m_sprites(),
      m_dirty()
{ }

gfx::anim::Controller::~Controller()
{ }

void
gfx::anim::Controller::addNewSprite(Sprite* p)
{
    // ex GfxSpriteManager3D::addSprite
    if (p != 0) {
        m_sprites.pushBackNew(p);
    }
}

void
gfx::anim::Controller::tick()
{
    // ex GfxSpriteManager3D::tick [sort-of]
    // Tick all sprites
    for (size_t i = 0, n = m_sprites.size(); i < n; ++i) {
        if (Sprite* p = m_sprites[i]) {
            p->tick();
        }
    }

    // Remove deleted sprites and collect dirty rectangles
    // FIXME: GfxSpriteManager3D jumped through some hoops to do multi-region clipping, claiming it is needed to bring FLAK to performance.
    // Do we still need that?
    //   SDL2: "chances are modern hardware can just swallow the whole framebuffer without much trouble"
    // A FullHD framebuffer is around 8 MByte.
    m_dirty = Rectangle();
    size_t out = 0;
    for (size_t i = 0, n = m_sprites.size(); i < n; ++i) {
        if (Sprite* p = m_sprites[i]) {
            // Add dirty region in any case
            m_dirty.include(p->getDirtyRegion());
            if (p->isMarkedForDeletion()) {
                // Deleted: add extent to cause other sprites or background to be redrawn
                m_dirty.include(p->getExtent());
            } else {
                // Clear its dirty region
                p->markClean();

                // Keep it by swapping to out.
                m_sprites.swapElements(out, i);
                ++out;
            }
        }
    }

    // Trim removed elements
    m_sprites.erase(m_sprites.begin() + out, m_sprites.end());

    // Sort remainder by Z
    m_sprites.sort(CompareSprites());
}

void
gfx::anim::Controller::draw(Canvas& can)
{
    // ex GfxSpriteManager3D::draw
    // Draw
    for (size_t i = 0, n = m_sprites.size(); i < n; ++i) {
        if (Sprite* p = m_sprites[i]) {
            p->draw(can);
        }
    }
}

const gfx::Rectangle&
gfx::anim::Controller::getDirtyRegion() const
{
    return m_dirty;
}

gfx::anim::Sprite*
gfx::anim::Controller::findSpriteById(int id) const
{
    for (size_t i = 0, n = m_sprites.size(); i < n; ++i) {
        if (Sprite* p = m_sprites[i]) {
            if (p->getId() == id && !p->isMarkedForDeletion()) {
                return p;
            }
        }
    }
    return 0;
}

void
gfx::anim::Controller::deleteSpritesById(int from, int to)
{
    for (size_t i = 0, n = m_sprites.size(); i < n; ++i) {
        if (Sprite* p = m_sprites[i]) {
            if (p->getId() >= from && p->getId() <= to && !p->isMarkedForDeletion()) {
                p->markForDeletion();
            }
        }
    }
}
