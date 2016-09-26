/**
  *  \file ui/rich/statictext.hpp
  */
#ifndef C2NG_UI_RICH_STATICTEXT_HPP
#define C2NG_UI_RICH_STATICTEXT_HPP

#include "util/rich/text.hpp"
#include "ui/rich/document.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace rich {

    class StaticText : public SimpleWidget {
     public:
        StaticText(const util::rich::Text& text, int width, gfx::ResourceProvider& provider);
        ~StaticText();

        void setText(const util::rich::Text& text);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        ui::rich::Document m_document;
        int m_width;
    };

} }

#endif
