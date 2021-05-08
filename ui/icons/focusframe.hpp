/**
  *  \file ui/icons/focusframe.hpp
  */
#ifndef C2NG_UI_ICONS_FOCUSFRAME_HPP
#define C2NG_UI_ICONS_FOCUSFRAME_HPP

#include "ui/icons/icon.hpp"

namespace ui { namespace icons {

    class FocusFrame : public Icon {
     public:
        FocusFrame(Icon& content);
        ~FocusFrame();

        bool setPad(int pad);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Icon& m_content;
        int m_pad;
    };

} }

#endif
