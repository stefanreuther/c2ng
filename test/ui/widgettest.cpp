/**
  *  \file test/ui/widgettest.cpp
  *  \brief Test for ui::Widget
  */

#include "ui/widget.hpp"
#include "afl/test/testrunner.hpp"

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
        virtual void handlePositionChange()
            { }
        virtual void handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
    };

    void doFocusTest(afl::test::Assert a, bool remove)
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
        a.check("01. child focused", child.hasState(ui::Widget::FocusedState));

        // Add another widget with child (popup window)
        {
            NullWidget window2;
            NullWidget child2;
            window2.addChild(child2, 0);
            root.addChild(window2, 0);
            window2.requestFocus();

            // Focus is transferred to the popup
            a.check("11. child focused", !child.hasState(ui::Widget::FocusedState));
            a.check("12. window focused", !window.hasState(ui::Widget::FocusedState));
            a.check("13. child2 focused", child2.hasState(ui::Widget::FocusedState));
            a.check("14. window2 focused", window2.hasState(ui::Widget::FocusedState));

            // If configured: be nice and deregister the window. Otherwise, rely on the destructor.
            if (remove) {
                root.removeChild(window2);
            }
        }

        // After the window died, previous window and child must have focus
        a.check("21. child focused", child.hasState(ui::Widget::FocusedState));
        a.check("22, window focused", window.hasState(ui::Widget::FocusedState));
    }

}

/** Test focus behaviour if a widget dies.
    The container must reliably find a new focused widget. */
AFL_TEST("ui.Widget:focus:widget-dies", a)
{
    doFocusTest(a, false);
}

/** Test focus behaviour if a widget is explicitly removed before it dies.
    The container must reliably find a new focused widget.
    This takes a different code path than testDeathFocus(). */
AFL_TEST("ui.Widget:focus:widget-removed", a)
{
    doFocusTest(a, true);
}
