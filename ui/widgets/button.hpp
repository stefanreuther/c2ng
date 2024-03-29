/**
  *  \file ui/widgets/button.hpp
  *  \brief Class ui::widgets::Button
  */
#ifndef C2NG_UI_WIDGETS_BUTTON_HPP
#define C2NG_UI_WIDGETS_BUTTON_HPP

#include "ui/widgets/basebutton.hpp"
#include "ui/icons/button.hpp"
#include "util/keystring.hpp"

namespace ui { namespace widgets {

    /** Push button
        This class provides the look and feel of a standard push-button.
        \see ui::icons::Button */
    class Button : public BaseButton {
     public:
        /** Constructor.
            \param text Text
            \param key  Key
            \param root Root (for skin colors, fonts, prefix argument) */
        Button(String_t text, util::Key_t key, ui::Root& root);

        /** Constructor.
            \param ks   Text and key
            \param root Root (for skin colors, fonts, prefix argument) */
        Button(const util::KeyString& ks, ui::Root& root);
        ~Button();

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

        /** Set compact mode.
            \param flag Flag to enable compact mode
            \see ui::icons::Button::setCompact */
        void setCompact(bool flag);

     private:
        ui::icons::Button m_icon;
    };

} }

#endif
