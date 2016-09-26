/**
  *  \file ui/spacer.hpp
  */
#ifndef C2NG_UI_SPACER_HPP
#define C2NG_UI_SPACER_HPP

#include "ui/simplewidget.hpp"

namespace ui {

    class Spacer : public SimpleWidget {
     public:
        Spacer();
        ~Spacer();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
    };

}

#endif
