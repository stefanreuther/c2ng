/**
  *  \file ui/widgets/textbutton.hpp
  */
#ifndef C2NG_UI_WIDGETS_TEXTBUTTON_HPP
#define C2NG_UI_WIDGETS_TEXTBUTTON_HPP

#include "ui/widgets/basebutton.hpp"
#include "gfx/fontrequest.hpp"
#include "ui/root.hpp"
#include "ui/icons/colortext.hpp"

namespace ui { namespace widgets {

    class TextButton : public BaseButton {
     public:
        TextButton(const String_t& text, util::Key_t key, Root& root);
        ~TextButton();

        void setText(const String_t& text);
        void setColor(uint8_t color);
        void setHoverColor(uint8_t color);
        void setFont(gfx::FontRequest font);
        void setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y);

     private:
        ui::icons::ColorText m_icon;
    };

} }

#endif
