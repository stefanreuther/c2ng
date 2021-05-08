/**
  *  \file ui/icons/image.hpp
  */
#ifndef C2NG_UI_ICONS_IMAGE_HPP
#define C2NG_UI_ICONS_IMAGE_HPP

#include "ui/icons/icon.hpp"

namespace ui { namespace icons {

    class Image : public Icon {
     public:
        Image(gfx::Point size);
        Image(afl::base::Ref<gfx::Canvas> image);
        ~Image();

        bool setImage(afl::base::Ptr<gfx::Canvas> image);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        const gfx::Point m_size;
        afl::base::Ptr<gfx::Canvas> m_image;
    };

} }

#endif
