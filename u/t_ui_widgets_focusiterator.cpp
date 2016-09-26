/**
  *  \file u/t_ui_widgets_focusiterator.cpp
  *  \brief Test for ui::widgets::FocusIterator
  */

#include "ui/widgets/focusiterator.hpp"

#include "t_ui_widgets.hpp"

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
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return defaultHandleMouse(pt, pressedButtons); }
    };

    struct TestBench {
        TestBench(int fiFlags)
            : root(),
              in1(),
              in2(),
              out(),
              in3(),
              it(fiFlags)
            {
                root.addChild(in1, 0);
                root.addChild(in2, 0);
                root.addChild(out, 0);
                root.addChild(in3, 0);
                root.addChild(it, 0);
                it.add(in1);
                it.add(in2);
                it.add(in3);
                root.requestFocus();
            }
        NullWidget root;
        NullWidget in1;
        NullWidget in2;
        NullWidget out;
        NullWidget in3;
        ui::widgets::FocusIterator it;
    };
}

/** Test basic tab behaviour. */
void
TestUiWidgetsFocusIterator::testTab()
{
    TestBench b(ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();

    // Verify pre-state
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Try tab
    TS_ASSERT(b.root.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    TS_ASSERT(b.root.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; wraps
    TS_ASSERT(b.root.handleKey(util::Key_Tab, 0));
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));
}

/** Test behaviour with empty FocusIterator.
    Must not deadlock / infinite loop. */
void
TestUiWidgetsFocusIterator::testEmpty()
{
    ui::widgets::FocusIterator testee(ui::widgets::FocusIterator::Tab);
    TS_ASSERT(!testee.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!testee.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour with FocusIterator and all-disabled widgets.
    Must not deadlock / infinite loop. */
void
TestUiWidgetsFocusIterator::testDisabled()
{
    TestBench b(ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);
    b.in2.setState(ui::Widget::DisabledState, true);
    b.in3.setState(ui::Widget::DisabledState, true);

    TS_ASSERT(!b.it.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!b.it.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour with FocusIterator, all-disabled widgets, and wrap.
    Must not deadlock / infinite loop. */
void
TestUiWidgetsFocusIterator::testDisabledWrap()
{
    TestBench b(ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Wrap);
    b.in1.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);
    b.in2.setState(ui::Widget::DisabledState, true);
    b.in3.setState(ui::Widget::DisabledState, true);

    TS_ASSERT(!b.it.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!b.it.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour of Home/End including disabled widgets. */
void
TestUiWidgetsFocusIterator::testDisabledHome()
{
    TestBench b(ui::widgets::FocusIterator::Home);
    b.in3.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);

    // Home goes to second widget (first is disabled)
    TS_ASSERT(b.it.handleKey(util::Key_Home, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // End goes to last widget
    TS_ASSERT(b.it.handleKey(util::Key_End, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement without wrap. */
void
TestUiWidgetsFocusIterator::testVertical()
{
    TestBench b(ui::widgets::FocusIterator::Vertical);
    b.in1.requestFocus();

    // Verify pre-state
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Try down
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; no change.
    TS_ASSERT(!b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement with wrap. */
void
TestUiWidgetsFocusIterator::testVerticalWrap()
{
    TestBench b(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Wrap);
    b.in1.requestFocus();

    // Verify pre-state
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Try tab
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; wraps
    TS_ASSERT( b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement in combination with Tab.
    Vertical movement does not wrap, Tab does. */
void
TestUiWidgetsFocusIterator::testVerticalTab()
{
    TestBench b(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();

    // Verify pre-state
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Try down
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    TS_ASSERT(b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; no change.
    TS_ASSERT(!b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));

    // Try tab, wraps.
    TS_ASSERT(b.root.handleKey(util::Key_Tab, 0));
    TS_ASSERT( b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));

    // Try shift-tab, wraps back.
    TS_ASSERT(b.root.handleKey(util::Key_Tab | util::KeyMod_Shift, 0));
    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.in3.hasState(ui::Widget::FocusedState));
}

/** Test behaviour if the focused widget is not one of ours. */
void
TestUiWidgetsFocusIterator::testOther()
{
    TestBench b(ui::widgets::FocusIterator::Horizontal | ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Page);
    b.out.requestFocus();

    TS_ASSERT(!b.root.handleKey(util::Key_Tab, 0));
    TS_ASSERT(!b.root.handleKey(util::Key_Up, 0));
    TS_ASSERT(!b.root.handleKey(util::Key_Down, 0));
    TS_ASSERT(!b.root.handleKey(util::Key_Left, 0));
    TS_ASSERT(!b.root.handleKey(util::Key_Right, 0));
    TS_ASSERT(!b.root.handleKey(util::Key_Home, 0));

    TS_ASSERT(!b.in1.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in2.hasState(ui::Widget::FocusedState));
    TS_ASSERT(!b.in3.hasState(ui::Widget::FocusedState));
    TS_ASSERT( b.out.hasState(ui::Widget::FocusedState));
}
