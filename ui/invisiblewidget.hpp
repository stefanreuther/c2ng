/**
  *  \file ui/invisiblewidget.hpp
  *  \brief Class ui::InvisibleWidget
  */
#ifndef C2NG_UI_INVISIBLEWIDGET_HPP
#define C2NG_UI_INVISIBLEWIDGET_HPP

#include "ui/simplewidget.hpp"

namespace ui {

    /** Base class for an invisible widget.
        This is a convenience class for implementing invisible widgets.

        An invisible widget only receives keyboard input.
        It has no shape (=no draw, getLayoutInfo), and therefore does not receive size and state notifications,
        nor mouse events. */
    class InvisibleWidget : public SimpleWidget {
     public:
        InvisibleWidget();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual ui::layout::Info getLayoutInfo() const;

        // virtual bool handleKey(util::Key_t key, int prefix); --> child

        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
    };

}

#endif
