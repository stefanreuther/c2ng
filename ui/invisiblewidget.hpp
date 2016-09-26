/**
  *  \file ui/invisiblewidget.hpp
  */
#ifndef C2NG_UI_INVISIBLEWIDGET_HPP
#define C2NG_UI_INVISIBLEWIDGET_HPP

#include "ui/simplewidget.hpp"

namespace ui {

    class InvisibleWidget : public SimpleWidget {
     public:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        // virtual bool handleKey(util::Key_t key, int prefix); --> child
        // virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons); --> child

    };

}

#endif
