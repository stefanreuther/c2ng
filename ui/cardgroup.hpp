/**
  *  \file ui/cardgroup.hpp
  *  \brief Class ui::CardGroup
  */
#ifndef C2NG_UI_CARDGROUP_HPP
#define C2NG_UI_CARDGROUP_HPP

#include "ui/widget.hpp"

namespace ui {

    /** \class CardGroup
        \brief Group containing widgets of which only one is visible at a time

        An CardGroup can contain a number of child widgets.
        Of these, only the focused widget is displayed; the others will be invisible and not receive events.

        In order to change the displayed widget, change the focus.
        CardGroup does not contain any special provisions to do that. */
    class CardGroup : public Widget {
     public:
        CardGroup() throw();
        ~CardGroup();

        void add(Widget& w);

        virtual void handleStateChange(State st, bool enable);

        // Redraw
        virtual void draw(gfx::Canvas& can);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);

        // Add
        virtual void handleChildAdded(Widget& child);

        // Remove
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        // Event
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
    };

}

#endif
