/**
  *  \file gfx/anim/sprite.hpp
  */
#ifndef C2NG_GFX_ANIM_SPRITE_HPP
#define C2NG_GFX_ANIM_SPRITE_HPP

#include "afl/base/deletable.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/canvas.hpp"

namespace gfx { namespace anim {

    class Sprite : public afl::base::Deletable {
     public:
        Sprite();

        void setExtent(const Rectangle& extent);
        const Rectangle& getExtent() const;

        void setCenter(Point pt);
        Point getCenter() const;

        void setZ(int z);
        int getZ() const;

        void setId(int id);
        int getId() const;

        void markChanged();
        void markClean();
        bool isChanged() const;
        const Rectangle& getDirtyRegion() const;

        void markForDeletion();
        bool isMarkedForDeletion() const;

        virtual void draw(Canvas& can) = 0;
        virtual void tick() = 0;

     private:
        Rectangle m_extent;
        Rectangle m_dirty;
        int m_z;
        int m_id;
        bool m_markedForDeletion;

    };

} }

#endif
