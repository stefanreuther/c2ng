/**
  *  \file u/t_ui_widget.cpp
  *  \brief Test for ui::Widget
  */

#include "ui/widget.hpp"

#include "t_ui.hpp"

namespace {
    class NullWidget : public ui::Widget {
     public:
        virtual void draw(gfx::Canvas& /*can*/)
            { }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& /*area*/)
            { }
        virtual void handleChildAdded(Widget& /*child*/)
            { }
        virtual void handleChildRemove(Widget& /*child*/)
            { }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { }
        virtual void handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
    };

    void doFocusTest(bool remove)
    {
        // Make a root widget
        NullWidget root;
        root.setState(ui::Widget::FocusedState, true);

        // Add a widget with child
        NullWidget window;
        NullWidget child;
        window.addChild(child, 0);
        root.addChild(window, 0);
        window.requestFocus();

        // Child must be focused now
        TS_ASSERT(child.hasState(ui::Widget::FocusedState));

        // Add another widget with child (popup window)
        {
            NullWidget window2;
            NullWidget child2;
            window2.addChild(child2, 0);
            root.addChild(window2, 0);
            window2.requestFocus();

            // Focus is transferred to the popup
            TS_ASSERT(!child.hasState(ui::Widget::FocusedState));
            TS_ASSERT(!window.hasState(ui::Widget::FocusedState));
            TS_ASSERT(child2.hasState(ui::Widget::FocusedState));
            TS_ASSERT(window2.hasState(ui::Widget::FocusedState));

            // If configured: be nice and deregister the window. Otherwise, rely on the destructor.
            if (remove) {
                root.removeChild(window2);
            }
        }

        // After the window died, previous window and child must have focus
        TS_ASSERT(child.hasState(ui::Widget::FocusedState));
        TS_ASSERT(window.hasState(ui::Widget::FocusedState));
    }

}

/** Test focus behaviour if a widget dies.
    The container must reliably find a new focused widget. */
void
TestUiWidget::testDeathFocus()
{
    doFocusTest(false);
}

/** Test focus behaviour if a widget is explicitly removed before it dies.
    The container must reliably find a new focused widget.
    This takes a different code path than testDeathFocus(). */
void
TestUiWidget::testRemoveFocus()
{
    doFocusTest(true);
}

