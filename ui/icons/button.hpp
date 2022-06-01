/**
  *  \file ui/icons/button.hpp
  *  \brief Class ui::icons::Button
  */
#ifndef C2NG_UI_ICONS_BUTTON_HPP
#define C2NG_UI_ICONS_BUTTON_HPP

#include "ui/icons/icon.hpp"
#include "ui/root.hpp"

namespace ui { namespace icons {

    /** Appearance of a push-button. */
    class Button : public Icon {
     public:
        /** Constructor.
            \param text Text
            \param font Font
            \param root Root (to obtain skin colors, fonts) */
        Button(String_t text, gfx::FontRequest font, Root& root);
        ~Button();

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

        /** Set font.
            For use during setup; font should not change during lifetime (as that would require re-layout).
            \param font New font request */
        void setFont(gfx::FontRequest font);

        /** Set alignment.
            \param x horizontal alignment
            \param y vertical alignment */
        void setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y);

        /** Set text.
            \param text Text */
        void setText(const String_t& text);

     private:
        String_t m_text;
        gfx::FontRequest m_font;
        gfx::HorizontalAlignment m_xAlign;
        gfx::VerticalAlignment m_yAlign;
        Root& m_root;
    };

} }

#endif
