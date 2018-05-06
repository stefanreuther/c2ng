/**
  *  \file u/t_ui_widgets_abstractbutton.cpp
  *  \brief Test for ui::widgets::AbstractButton
  */

#include "ui/widgets/abstractbutton.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"

/** Simple keyboard tests. */
void
TestUiWidgetsAbstractButton::testIt()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // A test class
    class Testee : public ui::widgets::AbstractButton {
     public:
        Testee(ui::Root& root, util::Key_t key)
            : AbstractButton(root, key)
            { }
        virtual void draw(gfx::Canvas& /*can*/)
            { }
        virtual void handleStateChange(State /*st*/, bool /*enable*/)
            { }
        virtual void handlePositionChange(gfx::Rectangle& /*oldPosition*/)
            { }
        virtual ui::layout::Info getLayoutInfo() const
            { return ui::layout::Info(); }
        virtual bool handleKey(util::Key_t key, int prefix)
            { return defaultHandleKey(key, prefix); }
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
            { return defaultHandleMouse(pt, pressedButtons); }
    };

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
        Testee t(root, 'a');
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
        Testee t(root, util::Key_F3);
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
        Testee t(root, '#');
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

