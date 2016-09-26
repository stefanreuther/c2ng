/**
  *  \file ui/widgets/button.hpp
  */
#ifndef C2NG_UI_WIDGETS_BUTTON_HPP
#define C2NG_UI_WIDGETS_BUTTON_HPP

#include "ui/widgets/abstractbutton.hpp"
#include "ui/colorscheme.hpp"

namespace ui { namespace widgets {

    class Button : public AbstractButton {
     public:
        Button(String_t text, util::Key_t key, gfx::ResourceProvider& provider, ui::ColorScheme& scheme);
        ~Button();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        void setFont(gfx::FontRequest font);

     private:
        String_t m_text;

        gfx::ResourceProvider& m_provider;
        ui::ColorScheme& m_scheme;
        gfx::FontRequest m_font;
    };

} }

#endif
