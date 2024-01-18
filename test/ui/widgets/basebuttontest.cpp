/**
  *  \file test/ui/widgets/basebuttontest.cpp
  *  \brief Test for ui::widgets::BaseButton
  */

#include "ui/widgets/basebutton.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

/** Simple keyboard tests. */
AFL_TEST("ui.widgets.BaseButton:handleKey", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // A test listener
    class Listener : public afl::base::Closure<void(int)> {
     public:
        Listener(int& counter)
            : m_counter(counter)
            { }
        virtual void call(int)
            { ++m_counter; }
        virtual Listener* clone() const
            { return new Listener(m_counter); }
     private:
        int& m_counter;
    };

    // Test with an alphabetic key
    {
        int counter = 0;
        ui::widgets::BaseButton t(root, 'a');
        t.sig_fire.addNewClosure(new Listener(counter));
        a.checkEqual("01", counter, 0);

        t.handleKey('a', 0);
        a.checkEqual("11", counter, 1);

        t.handleKey('A', 0);
        a.checkEqual("21", counter, 1);

        t.handleKey(util::KeyMod_Alt + 'a', 0);
        a.checkEqual("31", counter, 2);

        t.handleKey('a', 0);
        a.checkEqual("41", counter, 3);

        t.handleKey('#', 0);
        a.checkEqual("51", counter, 3);

        t.handleKey('\\', 0);
        a.checkEqual("61", counter, 3);
    }

    // Test with a function key
    {
        int counter = 0;
        ui::widgets::BaseButton t(root, util::Key_F3);
        t.sig_fire.addNewClosure(new Listener(counter));
        a.checkEqual("71", counter, 0);

        t.handleKey(util::Key_F3, 0);
        a.checkEqual("81", counter, 1);

        t.handleKey(util::Key_F3 + util::KeyMod_Shift, 0);
        a.checkEqual("91", counter, 1);

        t.handleKey(util::KeyMod_Alt + util::Key_F3, 0);
        a.checkEqual("101", counter, 2);

        t.handleKey(util::Key_F3, 0);
        a.checkEqual("111", counter, 3);

        t.handleKey('#', 0);
        a.checkEqual("121", counter, 3);

        t.handleKey('\\', 0);
        a.checkEqual("131", counter, 3);
    }

    // Test with "#"
    {
        int counter = 0;
        ui::widgets::BaseButton t(root, '#');
        t.sig_fire.addNewClosure(new Listener(counter));
        a.checkEqual("141", counter, 0);

        t.handleKey('#', 0);
        a.checkEqual("151", counter, 1);

        t.handleKey(util::KeyMod_Alt + '#', 0);
        a.checkEqual("161", counter, 2);

        t.handleKey('#', 0);
        a.checkEqual("171", counter, 3);

        t.handleKey('\\', 0);
        a.checkEqual("181", counter, 4);
    }
}
