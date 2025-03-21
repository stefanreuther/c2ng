/**
  *  \file test/ui/widgets/basebuttontest.cpp
  *  \brief Test for ui::widgets::BaseButton
  */

#include "ui/widgets/basebutton.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/icons/colortile.hpp"
#include "ui/root.hpp"

using gfx::EventConsumer;
using gfx::Point;
using gfx::Rectangle;
using ui::icons::ColorTile;
using ui::layout::Info;
using ui::widgets::BaseButton;

namespace {
    // A test listener
    class Listener : public afl::base::Closure<void(int)> {
     public:
        Listener(int& counter)
            : m_counter(counter)
            { }
        virtual void call(int)
            { ++m_counter; }
     private:
        int& m_counter;
    };

    // Test environment
    struct Environment {
        gfx::NullEngine engine;
        gfx::NullResourceProvider provider;
        ui::Root root;

        Environment()
            : engine(), provider(), root(engine, provider, gfx::WindowParameters())
            { }
    };
}

// Test with an alphabetic key
AFL_TEST("ui.widgets.BaseButton:handleKey:alphabetic", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, 'a');
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);
    a.checkEqual("02", t.getKey(), util::Key_t('a'));

    a.checkEqual("10", t.handleKey('a', 0), true);
    a.checkEqual("11", counter, 1);

    a.checkEqual("20", t.handleKey('A', 0), false);
    a.checkEqual("21", counter, 1);

    a.checkEqual("30", t.handleKey(util::KeyMod_Alt + 'a', 0), true);
    a.checkEqual("31", counter, 2);

    a.checkEqual("40", t.handleKey('a', 0), true);
    a.checkEqual("41", counter, 3);

    a.checkEqual("50", t.handleKey('#', 0), false);
    a.checkEqual("51", counter, 3);

    a.checkEqual("60", t.handleKey('\\', 0), false);
    a.checkEqual("61", counter, 3);

    a.checkEqual("70", t.handleKey(' ', 0), false);
    a.checkEqual("71", counter, 3);
}

// Test with an alphabetic key, disabled
AFL_TEST("ui.widgets.BaseButton:handleKey:alphabetic:disabled", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, 'a');
    t.sig_fire.addNewClosure(new Listener(counter));
    t.setState(BaseButton::DisabledState, true);
    a.checkEqual("01", counter, 0);
    a.checkEqual("02", t.getKey(), util::Key_t('a'));

    a.checkEqual("10", t.handleKey('a', 0), false);
    a.checkEqual("11", counter, 0);
}

// Test with a function key
AFL_TEST("ui.widgets.BaseButton:handleKey:function", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, util::Key_F3);
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);

    a.checkEqual("10", t.handleKey(util::Key_F3, 0), true);
    a.checkEqual("11", counter, 1);

    a.checkEqual("20", t.handleKey(util::Key_F3 + util::KeyMod_Shift, 0), false);
    a.checkEqual("21", counter, 1);

    a.checkEqual("30", t.handleKey(util::KeyMod_Alt + util::Key_F3, 0), true);
    a.checkEqual("31", counter, 2);

    a.checkEqual("40", t.handleKey(util::Key_F3, 0), true);
    a.checkEqual("41", counter, 3);

    a.checkEqual("50", t.handleKey('#', 0), false);
    a.checkEqual("51", counter, 3);

    a.checkEqual("60", t.handleKey('\\', 0), false);
    a.checkEqual("61", counter, 3);

    a.checkEqual("70", t.handleKey(' ', 0), false);
    a.checkEqual("71", counter, 3);
}

// Test with "#"
AFL_TEST("ui.widgets.BaseButton:handleKey:hash", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, '#');
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);

    a.checkEqual("10", t.handleKey('#', 0), true);
    a.checkEqual("11", counter, 1);

    a.checkEqual("20", t.handleKey(util::KeyMod_Alt + '#', 0), true);
    a.checkEqual("21", counter, 2);

    a.checkEqual("30", t.handleKey('#', 0), true);
    a.checkEqual("31", counter, 3);

    a.checkEqual("40", t.handleKey('\\', 0), true);
    a.checkEqual("41", counter, 4);

    a.checkEqual("50", t.handleKey(' ', 0), false);
    a.checkEqual("51", counter, 4);
}

// Test focusable button
AFL_TEST("ui.widgets.BaseButton:handleKey:focusable", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, 'x');
    t.setIsFocusable(true);
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);
    a.checkEqual("02", t.hasState(BaseButton::FocusedState), false);

    // Not yet focused; space is ignored
    a.checkEqual("10", t.handleKey(' ', 0), false);
    a.checkEqual("11", counter, 0);

    // Hotkey focuses
    a.checkEqual("20", t.handleKey('x', 0), true);
    a.checkEqual("21", counter, 1);
    a.checkEqual("22", t.hasState(BaseButton::FocusedState), true);

    // Space now fires the button
    a.checkEqual("30", t.handleKey(' ', 0), true);
    a.checkEqual("31", counter, 2);
}

// Test with click
AFL_TEST("ui.widgets.BaseButton:handleMouse:click", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, '#');
    t.setExtent(Rectangle(100, 200, 50, 30));
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);

    a.checkEqual("10", t.handleMouse(Point(105, 205), EventConsumer::MouseButtons_t() + EventConsumer::LeftButton), true);
    a.checkEqual("11", counter, 0);
    a.checkEqual("12", t.handleMouse(Point(105, 205), EventConsumer::MouseButtons_t()), true);
    a.checkEqual("13", counter, 1);
}

// Test move out
AFL_TEST("ui.widgets.BaseButton:handleMouse:move-out", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, '#');
    t.setExtent(Rectangle(100, 200, 50, 30));
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);

    a.checkEqual("10", t.handleMouse(Point(105, 205), EventConsumer::MouseButtons_t() + EventConsumer::LeftButton), true);
    a.checkEqual("11", counter, 0);
    a.checkEqual("12", t.handleMouse(Point(155, 205), EventConsumer::MouseButtons_t() + EventConsumer::LeftButton), false);
    a.checkEqual("13", counter, 0);
    a.checkEqual("14", t.handleMouse(Point(155, 205), EventConsumer::MouseButtons_t()), false);
    a.checkEqual("15", counter, 0);
}

// Test move in
AFL_TEST("ui.widgets.BaseButton:handleMouse:move-in", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, '#');
    t.setExtent(Rectangle(100, 200, 50, 30));
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);

    a.checkEqual("10", t.handleMouse(Point(95, 205), EventConsumer::MouseButtons_t() + EventConsumer::LeftButton), false);
    a.checkEqual("11", counter, 0);
    a.checkEqual("12", t.handleMouse(Point(105, 205), EventConsumer::MouseButtons_t() + EventConsumer::LeftButton), true);
    a.checkEqual("13", counter, 0);
    a.checkEqual("14", t.handleMouse(Point(105, 205), EventConsumer::MouseButtons_t()), true);
    a.checkEqual("15", counter, 1);
}

// Test setKey
AFL_TEST("ui.widgets.BaseButton:setKey", a)
{
    Environment env;
    int counter = 0;
    BaseButton t(env.root, 'a');
    t.sig_fire.addNewClosure(new Listener(counter));
    a.checkEqual("01", counter, 0);
    a.checkEqual("02", t.getKey(), util::Key_t('a'));

    a.checkEqual("10", t.handleKey('a', 0), true);
    a.checkEqual("11", counter, 1);

    // Change key
    t.setKey('b');
    a.checkEqual("20", t.getKey(), util::Key_t('b'));
    a.checkEqual("21", t.handleKey('a', 0), false);
    a.checkEqual("22", counter, 1);
    a.checkEqual("23", t.handleKey('b', 0), true);
    a.checkEqual("24", counter, 2);
}

// Test layout
AFL_TEST("ui.widgets.BaseButton:layout", a)
{
    Environment env;
    ColorTile content(env.root, Point(30, 10), 7);
    content.setFrameWidth(0);
    a.checkEqual("01. precondition", content.getSize().getX(), 30);
    a.checkEqual("02. precondition", content.getSize().getY(), 10);

    BaseButton t(env.root, 'a');
    t.setIcon(content);

    t.setGrowthBehaviour(Info::GrowHorizontal);

    Info result = t.getLayoutInfo();
    a.checkEqual("11. getPreferredSize", result.getPreferredSize().getX(), 30);
    a.checkEqual("12. getPreferredSize", result.getPreferredSize().getY(), 10);
    a.checkEqual("13. isGrowHorizontal", result.isGrowHorizontal(), true);
    a.checkEqual("14. isGrowVertical",   result.isGrowVertical(), false);
    a.checkEqual("15. isIgnored",        result.isIgnored(), false);
}
