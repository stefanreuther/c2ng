/**
  *  \file test/ui/widgets/inputlinetest.cpp
  *  \brief Test for ui::widgets::InputLine
  */

#include "ui/widgets/inputline.hpp"

#include "afl/test/testrunner.hpp"
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

        void verify(afl::test::Assert a, const String_t& expect)
            {
                a.checkEqual("getText", expect, m_widget.getText());
                a.checkEqual("lastValue", expect, m_lastValue);
            }

     private:
        ui::widgets::InputLine& m_widget;
        String_t m_lastValue;
    };
}

/** Test insertText(). */
AFL_TEST("ui.widgets.InputLine:insertText", a)
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
        a.checkEqual("01. getText", testee.getText(), "123456789a");
        a.checkEqual("02. getCursorIndex", testee.getCursorIndex(), 10U);
    }

    // Inserting overlong chunk
    {
        ui::widgets::InputLine testee(10, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText("123456789");
        testee.insertText("abcdefghijk");
        a.checkEqual("11. getText", testee.getText(), "123456789a");
        a.checkEqual("12. getCursorIndex", testee.getCursorIndex(), 10U);
    }

    // Test truncation with TypeErase=true
    {
        ui::widgets::InputLine testee(10, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText("123456789");
        testee.setFlag(ui::widgets::InputLine::TypeErase, true);
        testee.insertText("abcdefghijk");
        a.checkEqual("21. getText", testee.getText(), "abcdefghij");
        a.checkEqual("22. getCursorIndex", testee.getCursorIndex(), 10U);
    }

    // Test truncation with Unicode characters
    {
        ui::widgets::InputLine testee(3, root);
        testee.setFlag(ui::widgets::InputLine::TypeErase, false);
        testee.insertText(UTF_BULLET UTF_LEFT_ARROW);
        testee.insertText(UTF_RIGHT_ARROW UTF_UP_ARROW);
        a.checkEqual("31. getText", testee.getText(), UTF_BULLET UTF_LEFT_ARROW UTF_RIGHT_ARROW);
        a.checkEqual("32. getCursorIndex", testee.getCursorIndex(), 3U);
    }
}

/** Test handleKey(), in particular, change signalisation. */
AFL_TEST("ui.widgets.InputLine:handleKey", a)
{
    // Externals
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // ASCII insertion
    {
        ui::widgets::InputLine testee(10, root);
        Listener lis(testee);
        lis.verify(a("01. before"), "");
        a.check("02. handleKey", testee.handleKey('a', 0));
        lis.verify(a("03. after"), "a");
    }

    // Unicode insertion
    {
        ui::widgets::InputLine testee(10, root);
        Listener lis(testee);
        lis.verify(a("11. before"), "");
        a.check("12. handleKey", testee.handleKey(0x100, 0));
        lis.verify(a("13. after"), "\xC4\x80");
    }

    // Delete forward
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify(a("21. before"), "abc");
        a.check("22. handleKey", testee.handleKey(util::Key_Delete, 0));
        lis.verify(a("23. after"), "ac");
    }

    // Delete backward
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify(a("31. before"), "abc");
        a.check("32. handleKey", testee.handleKey(util::Key_Backspace, 0));
        lis.verify(a("33. after"), "bc");
    }

    // Delete all
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        testee.setCursorIndex(1);
        Listener lis(testee);
        lis.verify(a("41. before"), "abc");
        a.check("42. handleKey", testee.handleKey(util::KeyMod_Ctrl + 'y', 0));
        lis.verify(a("43. after"), "");
    }

    // Delete word
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc defg");
        testee.setCursorIndex(6);
        Listener lis(testee);
        lis.verify(a("51. before"), "abc defg");
        a.check("52. handleKey", testee.handleKey(util::KeyMod_Ctrl + util::Key_Backspace, 0));
        lis.verify(a("53. after"), "abc fg");
    }

    // Type-erase case
    {
        ui::widgets::InputLine testee(10, root);
        testee.setText("abc");
        Listener lis(testee);
        testee.setFlag(ui::widgets::InputLine::TypeErase, true);
        lis.verify(a("61. before"), "abc");
        a.check("62. handleKey", testee.handleKey('x', 0));
        a.check("63. handleKey", testee.handleKey('y', 0));
        lis.verify(a("64. after"), "xy");
    }

    // Non-printable
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, true);
        testee.setText("abc");
        a.check("71. handleKey", !testee.handleKey(3, 0));
        a.check("72. handleKey", !testee.handleKey(util::KeyMod_Alt, 0));
        a.checkEqual("73. getText", testee.getText(), "abc");
    }

    // Not focused
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, false);
        testee.setText("abc");
        a.check("81. handleKey", !testee.handleKey('x', 0));
        a.checkEqual("82. getText", testee.getText(), "abc");
    }

    // Activation
    {
        ui::widgets::InputLine testee(10, root);
        testee.setState(ui::Widget::FocusedState, false);
        testee.setText("abc");
        testee.setHotkey('y');
        a.check("91. handleKey", testee.handleKey('y', 0));
        a.checkEqual("92. getText", testee.getText(), "abc");
        a.check("93. FocusedState", testee.hasState(ui::Widget::FocusedState));

        a.check("94. handleKey", testee.handleKey('x', 0));
        a.checkEqual("95. getText", testee.getText(), "x");
    }
}
