/**
  *  \file u/t_ui_widgets_basebutton.cpp
  *  \brief Test for ui::widgets::BaseButton
  */

#include "ui/widgets/basebutton.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

/** Simple keyboard tests. */
void
TestUiWidgetsBaseButton::testKeyboard()
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
        TS_ASSERT_EQUALS(counter, 0);

        t.handleKey('a', 0);
        TS_ASSERT_EQUALS(counter, 1);

        t.handleKey('A', 0);
        TS_ASSERT_EQUALS(counter, 1);

        t.handleKey(util::KeyMod_Alt + 'a', 0);
        TS_ASSERT_EQUALS(counter, 2);

        t.handleKey('a', 0);
        TS_ASSERT_EQUALS(counter, 3);

        t.handleKey('#', 0);
        TS_ASSERT_EQUALS(counter, 3);

        t.handleKey('\\', 0);
        TS_ASSERT_EQUALS(counter, 3);
    }

    // Test with a function key
    {
        int counter = 0;
        ui::widgets::BaseButton t(root, util::Key_F3);
        t.sig_fire.addNewClosure(new Listener(counter));
        TS_ASSERT_EQUALS(counter, 0);

        t.handleKey(util::Key_F3, 0);
        TS_ASSERT_EQUALS(counter, 1);

        t.handleKey(util::Key_F3 + util::KeyMod_Shift, 0);
        TS_ASSERT_EQUALS(counter, 1);

        t.handleKey(util::KeyMod_Alt + util::Key_F3, 0);
        TS_ASSERT_EQUALS(counter, 2);

        t.handleKey(util::Key_F3, 0);
        TS_ASSERT_EQUALS(counter, 3);

        t.handleKey('#', 0);
        TS_ASSERT_EQUALS(counter, 3);

        t.handleKey('\\', 0);
        TS_ASSERT_EQUALS(counter, 3);
    }

    // Test with "#"
    {
        int counter = 0;
        ui::widgets::BaseButton t(root, '#');
        t.sig_fire.addNewClosure(new Listener(counter));
        TS_ASSERT_EQUALS(counter, 0);

        t.handleKey('#', 0);
        TS_ASSERT_EQUALS(counter, 1);

        t.handleKey(util::KeyMod_Alt + '#', 0);
        TS_ASSERT_EQUALS(counter, 2);

        t.handleKey('#', 0);
        TS_ASSERT_EQUALS(counter, 3);

        t.handleKey('\\', 0);
        TS_ASSERT_EQUALS(counter, 4);
    }
}

