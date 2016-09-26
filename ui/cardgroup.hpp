/**
  *  \file ui/cardgroup.hpp
  */
#ifndef C2NG_UI_CARDGROUP_HPP
#define C2NG_UI_CARDGROUP_HPP

#include "ui/widget.hpp"

namespace ui {

    class CardGroup : public Widget {
     public:
        CardGroup()
            : Widget()
            { }
        ~CardGroup()
            { }

        virtual void handleStateChange(State st, bool enable)
            {
                // FIXME
                (void) st; (void) enable;
            }

        // Redraw
        virtual void draw(gfx::Canvas& can)
            { if (Widget* w = getFocusedChild()) { w->draw(can); } }
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area)
            { if (getFocusedChild() == &child) { requestRedraw(area); } }

        // Add
        virtual void handleChildAdded(Widget& child)
            { child.setState(FocusedState, getFocusedChild() == &child); }

        // Remove
        virtual void handleChildRemove(Widget& /*child*/)
            { }

        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { }
        virtual void handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }

        // Event
        virtual bool handleKey(util::Key_t key, int prefix)
            { if (Widget* w = getFocusedChild()) { return w->handleKey(key, prefix); } else { return false; } }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { if (Widget* w = getFocusedChild()) { return w->handleMouse(pt, pressedButtons); } else { return false; } }
    };

}

#endif
