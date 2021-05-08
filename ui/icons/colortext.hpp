/**
  *  \file ui/icons/colortext.hpp
  */
#ifndef C2NG_UI_ICONS_COLORTEXT_HPP
#define C2NG_UI_ICONS_COLORTEXT_HPP

#include "ui/icons/icon.hpp"
#include "ui/root.hpp"

namespace ui { namespace icons {

    class ColorText : public Icon {
     public:
        ColorText(const String_t& text, Root& root);
        ~ColorText();

        bool setText(const String_t& text);
        bool setColor(uint8_t color);
        bool setHoverColor(uint8_t color);
        bool setFont(gfx::FontRequest font);
        bool setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Root& m_root;
        String_t m_text;
        uint8_t m_color;
        uint8_t m_hoverColor;
        gfx::FontRequest m_font;
        gfx::HorizontalAlignment m_alignX;
        gfx::VerticalAlignment m_alignY;
    };

} }

#endif
