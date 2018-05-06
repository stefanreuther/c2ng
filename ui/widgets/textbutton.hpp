/**
  *  \file ui/widgets/textbutton.hpp
  */
#ifndef C2NG_UI_WIDGETS_TEXTBUTTON_HPP
#define C2NG_UI_WIDGETS_TEXTBUTTON_HPP

#include "ui/widgets/abstractbutton.hpp"
#include "gfx/fontrequest.hpp"
#include "ui/root.hpp"

namespace ui { namespace widgets {

    class TextButton : public AbstractButton {
     public:
        TextButton(const String_t& text, util::Key_t key, Root& root);
        ~TextButton();

        void setText(const String_t& text);
        void setColor(uint8_t color);
        void setHoverColor(uint8_t color);
        void setFont(gfx::FontRequest font);
        void setTextAlign(int x, int y);

        // AbstractButton/Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        String_t m_text;
        uint8_t m_color;
        uint8_t m_hoverColor;
        gfx::FontRequest m_font;
        int m_alignX;
        int m_alignY;
    };

} }

#endif
