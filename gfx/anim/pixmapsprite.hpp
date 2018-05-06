/**
  *  \file gfx/anim/pixmapsprite.hpp
  */
#ifndef C2NG_GFX_ANIM_PIXMAPSPRITE_HPP
#define C2NG_GFX_ANIM_PIXMAPSPRITE_HPP

#include "gfx/anim/sprite.hpp"
#include "gfx/canvas.hpp"
#include "afl/base/ptr.hpp"

namespace gfx { namespace anim {

    class PixmapSprite : public Sprite {
     public:
        PixmapSprite(afl::base::Ptr<Canvas> pix);
        ~PixmapSprite();

        void setPixmap(afl::base::Ptr<Canvas> pix);

        virtual void draw(Canvas& can);
        virtual void tick();

     private:
        afl::base::Ptr<Canvas> m_pixmap;
    };

} }

#endif
