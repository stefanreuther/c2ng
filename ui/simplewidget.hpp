/**
  *  \file ui/simplewidget.hpp
  */
#ifndef C2NG_UI_SIMPLEWIDGET_HPP
#define C2NG_UI_SIMPLEWIDGET_HPP

#include "ui/widget.hpp"

namespace ui {

    class SimpleWidget : public Widget {
     public:
        SimpleWidget();
        ~SimpleWidget();

        // virtual void draw(gfx::Canvas& can); --> child
        // virtual void handleStateChange(State st, bool enable); --> child
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        // virtual void handlePositionChange(gfx::Rectangle& oldPosition); --> child
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        // virtual ui::layout::Info getLayoutInfo() const; --> child
        // virtual bool handleKey(util::Key_t key, int prefix); --> child
        // virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons); --> child
    };

}

#endif
