/**
  *  \file ui/rich/imageobject.hpp
  */
#ifndef C2NG_UI_RICH_IMAGEOBJECT_HPP
#define C2NG_UI_RICH_IMAGEOBJECT_HPP

#include "afl/base/ptr.hpp"
#include "gfx/canvas.hpp"
#include "ui/rich/blockobject.hpp"

namespace ui { namespace rich {

    class ImageObject : public BlockObject {
     public:
        ImageObject(afl::base::Ptr<gfx::Canvas> image);
        ~ImageObject();

        virtual gfx::Point getSize();
        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area);

     private:
        afl::base::Ptr<gfx::Canvas> m_image;
    };

} }

#endif
