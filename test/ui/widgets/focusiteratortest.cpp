/**
  *  \file test/ui/widgets/focusiteratortest.cpp
  *  \brief Test for ui::widgets::FocusIterator
  */

#include "ui/widgets/focusiterator.hpp"
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
AFL_TEST("ui.widgets.FocusIterator:Tab:normal", a)
{
    TestBench b(ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();

    // Verify pre-state
    a.check("01",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("02", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("03", !b.in3.hasState(ui::Widget::FocusedState));

    // Try tab
    a.check("11", b.root.handleKey(util::Key_Tab, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("14", !b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    a.check("21", b.root.handleKey(util::Key_Tab, 0));
    a.check("22", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("23", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("24",  b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; wraps
    a.check("31", b.root.handleKey(util::Key_Tab, 0));
    a.check("32",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("33", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("34", !b.in3.hasState(ui::Widget::FocusedState));
}

/** Test F6 behaviour. */
AFL_TEST("ui.widgets.FocusIterator:Tab:f6", a)
{
    TestBench b(ui::widgets::FocusIterator::F6);
    b.in1.requestFocus();

    // Verify pre-state
    a.check("01",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("02", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("03", !b.in3.hasState(ui::Widget::FocusedState));

    // Try F6
    a.check("11", b.root.handleKey(util::Key_F6, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("14", !b.in3.hasState(ui::Widget::FocusedState));

    // Back
    a.check("21", b.root.handleKey(util::Key_F6 | util::KeyMod_Shift, 0));
    a.check("22",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("23", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("24", !b.in3.hasState(ui::Widget::FocusedState));

    // Verify others
    a.check("91", !b.root.handleKey(util::Key_Tab, 0));
    a.check("92", !b.root.handleKey(util::Key_PgUp, 0));
    a.check("93", !b.root.handleKey(util::Key_Right, 0));
    a.check("94", !b.root.handleKey(util::Key_Down, 0));
    a.check("95",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("96", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("97", !b.in3.hasState(ui::Widget::FocusedState));
}

/** Test behaviour with empty FocusIterator.
    Must not deadlock / infinite loop. */
AFL_TEST("ui.widgets.FocusIterator:Tab:empty", a)
{
    ui::widgets::FocusIterator testee(ui::widgets::FocusIterator::Tab);
    a.check("01", !testee.handleKey(util::Key_Tab, 0));
    a.check("02", !testee.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour with FocusIterator and all-disabled widgets.
    Must not deadlock / infinite loop. */
AFL_TEST("ui.widgets.FocusIterator:Tab:all-disabled", a)
{
    TestBench b(ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);
    b.in2.setState(ui::Widget::DisabledState, true);
    b.in3.setState(ui::Widget::DisabledState, true);

    a.check("01", !b.it.handleKey(util::Key_Tab, 0));
    a.check("02", !b.it.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour with FocusIterator, all-disabled widgets, and wrap.
    Must not deadlock / infinite loop. */
AFL_TEST("ui.widgets.FocusIterator:Tab:all-disabled-wrap", a)
{
    TestBench b(ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Wrap);
    b.in1.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);
    b.in2.setState(ui::Widget::DisabledState, true);
    b.in3.setState(ui::Widget::DisabledState, true);

    a.check("01", !b.it.handleKey(util::Key_Tab, 0));
    a.check("02", !b.it.handleKey(util::Key_Tab + util::KeyMod_Shift, 0));
}

/** Test behaviour of Home/End including disabled widgets. */
AFL_TEST("ui.widgets.FocusIterator:Home:disabled", a)
{
    TestBench b(ui::widgets::FocusIterator::Home);
    b.in3.requestFocus();
    b.in1.setState(ui::Widget::DisabledState, true);

    // Home goes to second widget (first is disabled)
    a.check("01", b.it.handleKey(util::Key_Home, 0));
    a.check("02", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("03",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("04", !b.in3.hasState(ui::Widget::FocusedState));

    // End goes to last widget
    a.check("11", b.it.handleKey(util::Key_End, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("14",  b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement without wrap. */
AFL_TEST("ui.widgets.FocusIterator:Vertical", a)
{
    TestBench b(ui::widgets::FocusIterator::Vertical);
    b.in1.requestFocus();

    // Verify pre-state
    a.check("01",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("02", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("03", !b.in3.hasState(ui::Widget::FocusedState));

    // Try down
    a.check("11", b.root.handleKey(util::Key_Down, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("14", !b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    a.check("21", b.root.handleKey(util::Key_Down, 0));
    a.check("22", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("23", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("24",  b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; no change.
    a.check("31", !b.root.handleKey(util::Key_Down, 0));
    a.check("32", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("33", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("34",  b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement with wrap. */
AFL_TEST("ui.widgets.FocusIterator:Vertical:wrap", a)
{
    TestBench b(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Wrap);
    b.in1.requestFocus();

    // Verify pre-state
    a.check("01",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("02", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("03", !b.in3.hasState(ui::Widget::FocusedState));

    // Try tab
    a.check("11", b.root.handleKey(util::Key_Down, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("14", !b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    a.check("21", b.root.handleKey(util::Key_Down, 0));
    a.check("22", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("23", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("24",  b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; wraps
    a.check("31",  b.root.handleKey(util::Key_Down, 0));
    a.check("32",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("33", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("34", !b.in3.hasState(ui::Widget::FocusedState));
}

/** Test vertical movement in combination with Tab.
    Vertical movement does not wrap, Tab does. */
AFL_TEST("ui.widgets.FocusIterator:Vertical+Tab", a)
{
    TestBench b(ui::widgets::FocusIterator::Vertical | ui::widgets::FocusIterator::Tab);
    b.in1.requestFocus();

    // Verify pre-state
    a.check("01",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("02", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("03", !b.in3.hasState(ui::Widget::FocusedState));

    // Try down
    a.check("11", b.root.handleKey(util::Key_Down, 0));
    a.check("12", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("13",  b.in2.hasState(ui::Widget::FocusedState));
    a.check("14", !b.in3.hasState(ui::Widget::FocusedState));

    // Once more
    a.check("21", b.root.handleKey(util::Key_Down, 0));
    a.check("22", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("23", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("24",  b.in3.hasState(ui::Widget::FocusedState));

    // Reaches end; no change.
    a.check("31", !b.root.handleKey(util::Key_Down, 0));
    a.check("32", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("33", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("34",  b.in3.hasState(ui::Widget::FocusedState));

    // Try tab, wraps.
    a.check("41", b.root.handleKey(util::Key_Tab, 0));
    a.check("42",  b.in1.hasState(ui::Widget::FocusedState));
    a.check("43", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("44", !b.in3.hasState(ui::Widget::FocusedState));

    // Try shift-tab, wraps back.
    a.check("51", b.root.handleKey(util::Key_Tab | util::KeyMod_Shift, 0));
    a.check("52", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("53", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("54",  b.in3.hasState(ui::Widget::FocusedState));
}

/** Test behaviour if the focused widget is not one of ours. */
AFL_TEST("ui.widgets.FocusIterator:other-focus", a)
{
    TestBench b(ui::widgets::FocusIterator::Horizontal | ui::widgets::FocusIterator::Tab | ui::widgets::FocusIterator::Page);
    b.out.requestFocus();

    a.check("01", !b.root.handleKey(util::Key_Tab, 0));
    a.check("02", !b.root.handleKey(util::Key_Up, 0));
    a.check("03", !b.root.handleKey(util::Key_Down, 0));
    a.check("04", !b.root.handleKey(util::Key_Left, 0));
    a.check("05", !b.root.handleKey(util::Key_Right, 0));
    a.check("06", !b.root.handleKey(util::Key_Home, 0));

    a.check("11", !b.in1.hasState(ui::Widget::FocusedState));
    a.check("12", !b.in2.hasState(ui::Widget::FocusedState));
    a.check("13", !b.in3.hasState(ui::Widget::FocusedState));
    a.check("14",  b.out.hasState(ui::Widget::FocusedState));
}
