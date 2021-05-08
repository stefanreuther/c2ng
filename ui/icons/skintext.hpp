/**
  *  \file ui/icons/skintext.hpp
  */
#ifndef C2NG_UI_ICONS_SKINTEXT_HPP
#define C2NG_UI_ICONS_SKINTEXT_HPP

#include "ui/icons/icon.hpp"
#include "ui/root.hpp"

namespace ui { namespace icons {

    class SkinText : public Icon {
     public:
        SkinText(const String_t& text, Root& root);
        ~SkinText();

        bool setFont(gfx::FontRequest font);
        bool setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Root& m_root;
        String_t m_text;
        gfx::FontRequest m_font;
        gfx::HorizontalAlignment m_alignX;
        gfx::VerticalAlignment m_alignY;
    };

} }

#endif
