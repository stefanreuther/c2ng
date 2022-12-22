/**
  *  \file u/t_ui_widgets_inputline.cpp
  *  \brief Test for ui::widgets::InputLine
  */

#include "ui/widgets/inputline.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "ui/root.hpp"
#include "util/unicodechars.hpp"

namespace {
    class Listener {
     public:
        Listener(ui::widgets::InputLine& w)
            : m_widget(w)
            {
                m_widget.sig_change.add(this, &Listener::onChange);
                m_lastValue = m_widget.getText();

                // A sane default state
                m_widget.setState(ui::Widget::FocusedState, true);
                m_widget.setFlag(ui::widgets::InputLine::TypeErase, false);
            }

        void onChange()
            {
                m_lastValue = m_widget.getText();
            }

        void verify(const String_t& expect)
            {
                TS_ASSERT_EQUALS(expect, m_widget.getText());
                TS_ASSERT_EQUALS(expect, m_lastValue);
            }

     private:
        ui::widgets::InputLine& m_widget;
        String_t m_lastValue;
    };
}

/** Test insertText(). */
void
TestUiWidgetsInputLine::testInsert()
{
    // Externals
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Test truncation with TypeErase=0
    {
        ui::widgets::InputLine testee(10, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText("123456789");
        testee.insertText("abc");
        TS_ASSERT_EQUALS(testee.getText(), "123456789a");
        TS_ASSERT_EQUALS(testee.getCursorIndex(), 10U);
    }

    // Inserting overlong chunk
    {
        ui::widgets::InputLine testee(10, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText("123456789");
        testee.insertText("abcdefghijk");
        TS_ASSERT_EQUALS(testee.getText(), "123456789a");
        TS_ASSERT_EQUALS(testee.getCursorIndex(), 10U);
    }

    // Test truncation with TypeErase=true
    {
        ui::widgets::InputLine testee(10, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText("123456789");
        testee.setFlag(ui::widgets::InputLine::TypeErase, true);
        testee.insertText("abcdefghijk");
        TS_ASSERT_EQUALS(testee.getText(), "abcdefghij");
        TS_ASSERT_EQUALS(testee.getCursorIndex(), 10U);
    }

    // Test truncation with Unicode characters
    {
        ui::widgets::InputLine testee(3, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText(UTF_BULLET UTF_LEFT_ARROW);
        testee.insertText(UTF_RIGHT_ARROW UTF_UP_ARROW);
        TS_ASSERT_EQUALS(testee.getText(), UTF_BULLET UTF_LEFT_ARROW UTF_RIGHT_ARROW);
        TS_ASSERT_EQUALS(testee.getCursorIndex(), 3U);
    }
}

/** Test handleKey(), in particular, change signalisation. */
void
TestUiWidgetsInputLine::testHandleKey()
{
    // Externals
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // ASCII insertion
    {
        ui::widgets::InputLine testee(10, root);
        Listener lis(testee);
        lis.verify("");
        TS_ASSERT(testee.handleKey('a', 0));
        lis.verify("a");
    }

    // Unicode insertion
    {
        ui::widgets::InputLine testee(10, root);
        Listener lis(testee);
        lis.verify("");
        TS_ASSERT(testee.handleKey(0x100, 0));
        lis.verify("\xC4\x80");
    }

    // Delete forward
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify("abc");
        TS_ASSERT(testee.handleKey(util::Key_Delete, 0));
        lis.verify("ac");
    }

    // Delete backward
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify("abc");
        TS_ASSERT(testee.handleKey(util::Key_Backspace, 0));
        lis.verify("bc");
    }

    // Delete all
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify("abc");
        TS_ASSERT(testee.handleKey(util::KeyMod_Ctrl + 'y', 0));
        lis.verify("");
    }

    // Delete word
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc defg");
        testee.setCursorIndex(6);
        Listener lis(testee);
        lis.verify("abc defg");
        TS_ASSERT(testee.handleKey(util::KeyMod_Ctrl + util::Key_Backspace, 0));
        lis.verify("abc fg");
    }

    // Type-erase case
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        Listener lis(testee);
        testee.setFlag(ui::widgets::InputLine::TypeErase, true);
        lis.verify("abc");
        TS_ASSERT(testee.handleKey('x', 0));
        TS_ASSERT(testee.handleKey('y', 0));
        lis.verify("xy");
    }

    // Non-printable
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, true);
        testee.setText("abc");
        TS_ASSERT(!testee.handleKey(3, 0));
        TS_ASSERT(!testee.handleKey(util::KeyMod_Alt, 0));
        TS_ASSERT_EQUALS(testee.getText(), "abc");
    }

    // Not focused
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, false);
        testee.setText("abc");
        TS_ASSERT(!testee.handleKey('x', 0));
        TS_ASSERT_EQUALS(testee.getText(), "abc");
    }

    // Activation
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, false);
        testee.setText("abc");
        testee.setHotkey('y');
        TS_ASSERT(testee.handleKey('y', 0));
        TS_ASSERT_EQUALS(testee.getText(), "abc");
        TS_ASSERT(testee.hasState(ui::Widget::FocusedState));

        TS_ASSERT(testee.handleKey('x', 0));
        TS_ASSERT_EQUALS(testee.getText(), "x");
    }
}

