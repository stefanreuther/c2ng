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

/** Test insertText(). */
void
TestUiWidgetsInputLine::testInsert()
{
    // Externals
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, 100, 100, 8, gfx::Engine::WindowFlags_t());

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
