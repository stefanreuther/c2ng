/**
  *  \file gfx/anim/controller.hpp
  */
#ifndef C2NG_GFX_ANIM_CONTROLLER_HPP
#define C2NG_GFX_ANIM_CONTROLLER_HPP

#include "afl/container/ptrvector.hpp"
#include "gfx/anim/sprite.hpp"

namespace gfx { namespace anim {

    class Controller {
     public:
        Controller();
        ~Controller();

        template<typename T> T* addNew(T* p);

        void addNewSprite(Sprite* p);

        void tick();
        void draw(Canvas& can);
        const Rectangle& getDirtyRegion() const;

        Sprite* findSpriteById(int id) const;

     private:
        afl::container::PtrVector<Sprite> m_sprites;
        Rectangle m_dirty;
    };

} }

template<typename T>
inline T*
gfx::anim::Controller::addNew(T* p)
{
    addNewSprite(p);
    return p;
}

#endif
