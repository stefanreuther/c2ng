/**
  *  \file test/ui/widgets/numberselectortest.cpp
  *  \brief Test for ui::widgets::NumberSelector
  */

#include "ui/widgets/numberselector.hpp"

#include "afl/test/testrunner.hpp"

using afl::base::Observable;

namespace {
    class Tester : public ui::widgets::NumberSelector {
     public:
        Tester(Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
            : NumberSelector(value, min, max, step)
            { }
        bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*btn*/)
            { return false; }
        void draw(gfx::Canvas& /*can*/)
            { }
        void handleStateChange(State /*st*/, bool /*enabled*/)
            { }
        void handlePositionChange()
            { }
        ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
    };

    void testKey(afl::test::Assert a, util::Key_t key, int prefix, int from, int to)
    {
        Observable<int32_t> value;
        Tester t(value, 0, 1000, 10);
        t.requestFocus();
        t.setValue(from);
        a.checkEqual("key handled", t.handleKey(key, prefix), true);
        a.checkEqual("result value", t.getValue(), to);
    }
}

// Normal getter/setter test
AFL_TEST("ui.widgets.NumberSelector:setValue", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 100, 10);

    // Inquiry
    a.checkEqual("01. min", t.getMin(), 0);
    a.checkEqual("02. max", t.getMax(), 100);
    a.checkEqual("03. step", t.getStep(), 10);

    // Normal
    t.setValue(5);
    a.checkEqual("11. get", t.getValue(), 5);
    a.checkEqual("12. get", t.value().get(), 5);

    // Overflow
    t.setValue(200);
    a.checkEqual("21. get", t.getValue(), 100);
    a.checkEqual("22. get", t.value().get(), 100);

    // Underflow
    t.setValue(-50);
    a.checkEqual("31. get", t.getValue(), 0);
    a.checkEqual("32. get", t.value().get(), 0);
}

// increment(), normal behaviour
AFL_TEST("ui.widgets.NumberSelector:increment", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 100, 10);

    // Normal
    t.setValue(80);
    t.increment(15);
    a.checkEqual("01. get", t.getValue(), 95);
    a.checkEqual("02. get", t.value().get(), 95);

    // Hits maximum
    t.increment(15);
    a.checkEqual("11. get", t.getValue(), 100);
    a.checkEqual("12. get", t.value().get(), 100);
}

// increment(), zero (replaced by 1)
AFL_TEST("ui.widgets.NumberSelector:increment:zero", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 100, 10);

    // Normal
    t.setValue(80);
    t.increment(0);
    a.checkEqual("01. get", t.getValue(), 81);
    a.checkEqual("02. get", t.value().get(), 81);
}

// increment(), integer overflow
AFL_TEST("ui.widgets.NumberSelector:increment:int-overflow", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 0x7FFFFFFF, 10);

    // Normal
    t.setValue(0x7FFFFF00);
    t.increment(1000);
    a.checkEqual("01. get", t.getValue(), 0x7FFFFFFF);
    a.checkEqual("02. get", t.value().get(), 0x7FFFFFFF);
}

// decrement(), normal behaviour)
AFL_TEST("ui.widgets.NumberSelector:decrement", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 100, 10);

    // Normal
    t.setValue(20);
    t.decrement(15);
    a.checkEqual("01. get", t.getValue(), 5);
    a.checkEqual("02. get", t.value().get(), 5);

    // Hits maximum
    t.decrement(15);
    a.checkEqual("11. get", t.getValue(), 0);
    a.checkEqual("12. get", t.value().get(), 0);
}

// decrement(), zero (replaced by 1)
AFL_TEST("ui.widgets.NumberSelector:decrement:zero", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 100, 10);

    // Normal
    t.setValue(80);
    t.decrement(0);
    a.checkEqual("01. get", t.getValue(), 79);
    a.checkEqual("02. get", t.value().get(), 79);
}

// decrement(), integer overflow
AFL_TEST("ui.widgets.NumberSelector:decrement:int-overflow", a)
{
    const int32_t MIN = static_cast<int32_t>(-0x80000000);
    Observable<int32_t> value;
    Tester t(value, MIN, 0x7FFFFFFF, 10);

    // Normal
    t.setValue(-0x7FFFFF00);
    t.decrement(1000);
    a.checkEqual("01. get", t.getValue(), MIN);
    a.checkEqual("02. get", t.value().get(), MIN);
}

// handleKey() / defaultHandleKey()
AFL_TEST("ui.widgets.NumberSelector:handleKey", a)
{
    // Normal increment
    testKey(a("right"),           util::Key_Right,                      0, 30, 40);
    testKey(a("right pfx"),       util::Key_Right,                      7, 30, 37);
    testKey(a("+"),               '+',                                  0, 30, 40);
    testKey(a("+ pfx"),           '+',                                  7, 30, 37);
    testKey(a("+ pfx2"),          '+',                               9999, 30, 1000);

    // Normal decrement
    testKey(a("left"),            util::Key_Left,                       0, 30, 20);
    testKey(a("left pfx"),        util::Key_Left,                       7, 30, 23);
    testKey(a("-"),               '-',                                  0, 30, 20);
    testKey(a("- pfx"),           '-',                                  7, 30, 23);
    testKey(a("- pfx2"),          '-',                               9999, 30, 0);

    // With Ctrl (step of 100)
    testKey(a("ctrl right"),      util::KeyMod_Ctrl + util::Key_Right,  0, 30, 130);
    testKey(a("ctrl right pfx"),  util::KeyMod_Ctrl + util::Key_Right,  7, 30, 130);
    testKey(a("ctrl +"),          util::KeyMod_Ctrl + '+',              0, 30, 130);
    testKey(a("ctrl + pfx"),      util::KeyMod_Ctrl + '+',              7, 30, 130);

    testKey(a("ctrl left"),       util::KeyMod_Ctrl + util::Key_Left,   0, 430, 330);
    testKey(a("ctrl left pfx"),   util::KeyMod_Ctrl + util::Key_Left,   7, 430, 330);
    testKey(a("ctrl -"),          util::KeyMod_Ctrl + '-',              0, 430, 330);
    testKey(a("ctrl - pfx"),      util::KeyMod_Ctrl + '-',              7, 430, 330);

    // With Shift (step of 1d00)
    testKey(a("shift right"),     util::KeyMod_Shift + util::Key_Right, 0, 30, 31);
    testKey(a("shift right pfx"), util::KeyMod_Shift + util::Key_Right, 7, 30, 31);
    testKey(a("shift +"),         util::KeyMod_Shift + '+',             0, 30, 31);
    testKey(a("shift + pfx"),     util::KeyMod_Shift + '+',             7, 30, 31);

    testKey(a("shift left"),      util::KeyMod_Shift + util::Key_Left,  0, 430, 429);
    testKey(a("shift left pfx"),  util::KeyMod_Shift + util::Key_Left,  7, 430, 429);
    testKey(a("shift -"),         util::KeyMod_Shift + '-',             0, 430, 429);
    testKey(a("shift - pfx"),     util::KeyMod_Shift + '-',             7, 430, 429);

    // With Alt (max)
    testKey(a("alt right"),       util::KeyMod_Alt + util::Key_Right,   0, 30, 1000);
    testKey(a("alt right pfx"),   util::KeyMod_Alt + util::Key_Right,   7, 30, 1000);
    testKey(a("alt +"),           util::KeyMod_Alt + '+',               0, 30, 1000);
    testKey(a("alt + pfx"),       util::KeyMod_Alt + '+',               7, 30, 1000);

    testKey(a("alt left"),        util::KeyMod_Alt + util::Key_Left,    0, 430, 0);
    testKey(a("alt left pfx"),    util::KeyMod_Alt + util::Key_Left,    7, 430, 0);
    testKey(a("alt -"),           util::KeyMod_Alt + '-',               0, 430, 0);
    testKey(a("alt - pfx"),       util::KeyMod_Alt + '-',               7, 430, 0);
}

// handleKey() / defaultHandleKey(), other key
AFL_TEST("ui.widgets.NumberSelector:handleKey:other", a)
{
    Observable<int32_t> value;
    Tester t(value, 0, 1000, 10);
    t.requestFocus();
    a.checkEqual("key handled", t.handleKey('x', 99), false);
}
