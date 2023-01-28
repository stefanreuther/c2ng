/**
  *  \file ui/widgets/scrollbarcontainer.hpp
  *  \brief Class ui::widgets::ScrollbarContainer
  */
#ifndef C2NG_UI_WIDGETS_SCROLLBARCONTAINER_HPP
#define C2NG_UI_WIDGETS_SCROLLBARCONTAINER_HPP

#include "ui/widget.hpp"
#include "ui/widgets/scrollbar.hpp"
#include "afl/base/signalconnection.hpp"

namespace ui { namespace widgets {

    /** Container for a list with optional scrollbar.
        If the contained widget requires a scrollbar, shows one; otherwise, hides it.

        Layout computations always include the scrollbar:
        the contained widget is enlarged appropriately if no scrollbar is in use,
        the ScrollbarContainer does not dynamically grow or shrink. */
    class ScrollbarContainer : public Widget {
     public:
        /** Constructor.
            \param widget ScrollableWidget
            \param root   UI root (required for drawing the scrollbar) */
        ScrollbarContainer(ScrollableWidget& widget, Root& root);
        ~ScrollbarContainer();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        void onChange();
        void doLayout();

        ScrollableWidget& m_widget;
        Scrollbar m_scrollbar;
        bool m_hasScrollbar;
        afl::base::SignalConnection conn_change;
    };

} }

#endif
