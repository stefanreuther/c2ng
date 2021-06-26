/**
  *  \file u/t_util_editor_editor.cpp
  *  \brief Test for util::editor::Editor
  */

#include "util/editor/editor.hpp"

#include "t_util_editor.hpp"

namespace ed = util::editor;

/** Test basic configuration.
    A: call setCursor(), setLineLimit(), setLengthLimit().
    E: verify getters */
void
TestUtilEditorEditor::testConfig()
{
    util::editor::Editor t;
    TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    TS_ASSERT(t.getLineLimit() > 1000U);
    TS_ASSERT(t.getLengthLimit() > 1000U);

    t.setCursor(7, 9);
    t.setLineLimit(12);
    t.setLengthLimit(32);

    TS_ASSERT_EQUALS(t.getCurrentLine(), 7U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 9U);
    TS_ASSERT_EQUALS(t.getLineLimit(), 12U);
    TS_ASSERT_EQUALS(t.getLengthLimit(), 32U);
}

/** Test basic text handling.
    A: call setLine()
    E: verify getLine(), getRange() */
void
TestUtilEditorEditor::testText()
{
    util::editor::Editor t;
    t.setLine(2, "hello");
    t.setLine(3, "world");

    TS_ASSERT_EQUALS(t.getLineText(0), "");
    TS_ASSERT_EQUALS(t.getLineText(1), "");
    TS_ASSERT_EQUALS(t.getLineText(2), "hello");
    TS_ASSERT_EQUALS(t.getLineText(3), "world");
    TS_ASSERT_EQUALS(t.getLineText(4), "");

    TS_ASSERT_EQUALS(t.getRange(0, 0, 7, 0), "\n\nhello\nworld\n\n\n\n");
    TS_ASSERT_EQUALS(t.getRange(0, 10, 0, 20), "");
    TS_ASSERT_EQUALS(t.getRange(2, 1, 2, 3), "el");
    TS_ASSERT_EQUALS(t.getRange(2, 1, 3, 0), "ello\n");
    TS_ASSERT_EQUALS(t.getRange(2, 1, 3, 2), "ello\nwo");
    TS_ASSERT_EQUALS(t.getRange(2, 10, 3, 0), "\n");

    // Invalid
    TS_ASSERT_EQUALS(t.getRange(2, 3, 2, 1), "");
    TS_ASSERT_EQUALS(t.getRange(2, 3, 1, 0), "");
}

/** Test cMoveLineUp command. */
void
TestUtilEditorEditor::testCommandMoveLineUp()
{
    // Normal
    {
        util::editor::Editor t;
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 9U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Normal, AllowCursorAfterEnd
    {
        util::editor::Editor t;
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveLineUp), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 9U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }

    // At beginning
    {
        util::editor::Editor t;
        t.setCursor(0, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }

    // With restriction
    {
        util::editor::Editor t;
        t.setUserLineLimit(5, 10);
        t.setCursor(5, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 5U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }
}

/** Test cMoveLineDown command. */
void
TestUtilEditorEditor::testCommandMoveLineDown()
{
    // Normal
    {
        util::editor::Editor t;
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 11U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Normal, AllowCursorAfterEnd
    {
        util::editor::Editor t;
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveLineDown), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 11U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }

    // At end
    {
        util::editor::Editor t;
        t.setLineLimit(10);
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }

    // With restriction
    {
        util::editor::Editor t;
        t.setUserLineLimit(5, 10);
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }
}

/** Test cMoveCharacterLeft command. */
void
TestUtilEditorEditor::testCommandMoveCharacterLeft()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(10, "123456789");
        t.setCursor(10, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveCharacterLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);
    }

    // At beginning
    {
        util::editor::Editor t;
        t.setLine(10, "123456789");
        t.setCursor(10, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveCharacterLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }
}

/** Test cMoveCharacterRight command. */
void
TestUtilEditorEditor::testCommandMoveCharacterRight()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(10, "123456789");
        t.setCursor(10, 5);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveCharacterRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
    }

    // At end
    {
        util::editor::Editor t;
        t.setLine(10, "12345");
        t.setLengthLimit(5);
        t.setCursor(10, 5);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveCharacterRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 10U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
    }
}

/** Test cMoveWordLeft command. */
void
TestUtilEditorEditor::testCommandMoveWordLeft()
{
    // Sequence
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");

        // Start at "a<m>et".
        t.setCursor(1, 7);

        // Go to "<a>met"
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);

        // Go to "<s>it"
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);

        // Go to beginning of line.
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);

        // Go to "<d>olor".
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 12U);
    }

    // With limit
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "sit amet.");
        t.setUserLineLimit(1, 10);
        t.setCursor(1, 0);

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }
}

/** Test cMoveWordRight command. */
void
TestUtilEditorEditor::testCommandMoveWordRight()
{
    // Sequence
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");

        // Start at "i<p>sum".
        t.setCursor(0, 7);

        // Go end of "ipsum"
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 11U);

        // Go end of "dolor"
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 17U);

        // Go end of line
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 18U);

        // Go to next line
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
    }

    // Limit
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setUserLineLimit(0, 0);
        t.setCursor(0, 18);

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 18U);
    }
}

/** Test cMoveBeginningOfLine command. */
void
TestUtilEditorEditor::testCommandMoveBeginningOfLine()
{
    util::editor::Editor t;
    t.setLine(3, "Subject: hi.", 9, false);
    t.setCursor(3, 11);

    // Go to beginning of editable
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfLine), true);
    TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 9U);

    // Go to beginning of line
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfLine), true);
    TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
}

/** Test cMoveEndOfLine command. */
void
TestUtilEditorEditor::testCommandMoveEndOfLine()
{
    util::editor::Editor t;
    t.setLine(3, "Subject: hi.", 9, false);
    t.setCursor(3, 11);

    // Go to end
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveEndOfLine), true);
    TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 12U);
}

/** Test cMoveBeginningOfDocument command. */
void
TestUtilEditorEditor::testCommandMoveBeginningOfDocument()
{
    // Empty
    {
        util::editor::Editor t;
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Populated
    {
        util::editor::Editor t;
        t.setLine(0, "FROM: me", 1000, false);
        t.setLine(1, "TO: them", 1000, false);
        t.setLine(2, "Subject: hi.", 9, false);
        t.setLine(3, "hi");
        t.setLine(4, "there");

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 2U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 9U);
    }

    // Populated, Unicode
    {
        util::editor::Editor t;
        t.setLine(0, "\xC2\xA1\xC2\xA2\xC2\xA3", 4, false);
        t.setLine(1, "hi");
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Line limit
    {
        util::editor::Editor t;
        t.setLine(0, "a", 0, false);
        t.setLine(1, "b", 1000, false);
        t.setLine(2, "c", 1000, false);
        t.setLine(3, "hi");
        t.setLine(4, "there");
        t.setUserLineLimit(2, 100);

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }
}

/** Test cMoveEndOfDocument command. */
void
TestUtilEditorEditor::testCommandMoveEndOfDocument()
{
    // Empty
    {
        util::editor::Editor t;
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Populated
    {
        util::editor::Editor t;
        t.setLine(0, "FROM: me", 1000, false);
        t.setLine(1, "TO: them", 1000, false);
        t.setLine(2, "Subject: hi.", 9, false);
        t.setLine(3, "hi");
        t.setLine(4, "there");

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 4U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
    }

    // Limit
    {
        util::editor::Editor t;
        t.setLine(0, "a");
        t.setLine(1, "b");
        t.setLine(2, "c");
        t.setLine(3, "d");
        t.setLine(4, "e");
        t.setUserLineLimit(0, 3);

        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 1U);
    }
}

/** Test cDeleteCharacter command. */
void
TestUtilEditorEditor::testCommandDeleteCharacter()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hell");
        TS_ASSERT_EQUALS(t.getLineText(1), "there");
    }

    // Protected
    {
        util::editor::Editor t;
        t.setLine(0, "hi: there", 4, false);
        t.setCursor(0, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hi: there");
    }

    // Joining lines
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 5);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hellothere");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
    }

    // Joining lines, cursor after end
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello  there");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
    }

    // Joining lines, with wrap
    {
        util::editor::Editor t;
        //            123456789012345678901234567890
        t.setLine(0, "Duis sem velit, ultrices ");
        t.setLine(1, "et, fermentum auctor, rhoncus ut.");
        t.setCursor(0, 25);
        t.setLengthLimit(30);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Duis sem velit, ultrices et,");
        TS_ASSERT_EQUALS(t.getLineText(1), "fermentum auctor, rhoncus ut.");
    }

    // Joining lines, making long word
    {
        util::editor::Editor t;
        t.setLine(0, "One two");
        t.setLine(1, "three four");
        t.setCursor(0, 7);
        t.setLengthLimit(9);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "One");
        TS_ASSERT_EQUALS(t.getLineText(1), "twothree");
        TS_ASSERT_EQUALS(t.getLineText(2), "four");
    }

    // Next line protected
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there", 1, true);
        t.setCursor(0, 5);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "there");
    }
}

/** Test cDeleteCharacterBackward command. */
void
TestUtilEditorEditor::testCommandDeleteCharacterBackward()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(1, 4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "thee");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    }

    // With overwrite - FIXME: not implemented in single-line handler
    // {
    //     util::editor::Editor t;
    //     t.setLine(0, "hello");
    //     t.setLine(1, "there");
    //     t.setCursor(1, 4);
    //     TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(ed::Overwrite), ed::cDeleteCharacterBackward), true);
    //     TS_ASSERT_EQUALS(t.getLineText(0), "hello");
    //     TS_ASSERT_EQUALS(t.getLineText(1), "the e");
    //     TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
    //     TS_ASSERT_EQUALS(t.getCurrentColumn(), 3U);
    // }

    // Protected
    {
        util::editor::Editor t;
        t.setLine(0, "hi: there", 4, false);
        t.setCursor(0, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hi: there");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 1U);
    }

    // Cursor after end
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "there");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
    }

    // Joining lines
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(1, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hellothere");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
    }

    // Joining lines, with wrap
    {
        util::editor::Editor t;
        //            123456789012345678901234567890
        t.setLine(0, "Duis sem velit, ultrices ");
        t.setLine(1, "et, fermentum auctor, rhoncus ut.");
        t.setCursor(1, 0);
        t.setLengthLimit(30);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Duis sem velit, ultrices et,");
        TS_ASSERT_EQUALS(t.getLineText(1), "fermentum auctor, rhoncus ut.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 25U);
    }

    // Joining lines, limit
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(1, 0);
        t.setUserLineLimit(1, 10);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "there");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Previous line protected
    {
        util::editor::Editor t;
        t.setLine(0, "hello", 1000, true);
        t.setLine(1, "there");
        t.setCursor(1, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "there");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }
}

/** Test cDeleteLine command. */
void
TestUtilEditorEditor::testCommandDeleteLine()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setLine(2, "three");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "three");
        TS_ASSERT_EQUALS(t.getLineText(2), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Protected line
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two: half", 4, false);
        t.setLine(2, "three");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "two:");
        TS_ASSERT_EQUALS(t.getLineText(2), "three");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U); // end of protected area
    }
}

/** Test cDeleteEndOfLine command. */
void
TestUtilEditorEditor::testCommandDeleteEndOfLine()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(1, 4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello");
        TS_ASSERT_EQUALS(t.getLineText(1), "ther");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
    }

    // Protected
    {
        util::editor::Editor t;
        t.setLine(0, "hi: there", 4, false);
        t.setCursor(0, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hi: ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
    }

    // Cursor after end
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hello  there");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 7U);
    }

    // Joining lines
    {
        util::editor::Editor t;
        t.setLine(0, "hello");
        t.setLine(1, "there");
        t.setCursor(0, 5);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "hellothere");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
    }
}

/** Test cDeleteWordBackward command. */
void
TestUtilEditorEditor::testCommandDeleteWordBackward()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(1, 4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(1), "  t amet.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);
    }

    // Beginning
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(1), "sit amet.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }

    // Join lines
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(1, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Lorem ipsum   sit amet.");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 12U);
    }

    // Joining lines, with wrap
    {
        util::editor::Editor t;
        //            123456789012345678901234567890
        t.setLine(0, "Duis sem velit, ultrices ");
        t.setLine(1, "et, fermentum auctor, rhoncus ut.");
        t.setCursor(1, 0);
        t.setLengthLimit(30);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Duis sem velit, et, fermentum");
        TS_ASSERT_EQUALS(t.getLineText(1), "auctor, rhoncus ut.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 16U);
    }

    // Join lines, with re-wrap
    {
        util::editor::Editor t;
        t.setLine(0, "one two", 0, true);
        t.setLine(1, "three four", 0, true);
        t.setLine(2, "five six", 0, true);
        t.setLine(3, "sevn eight", 0, false);
        t.setLine(4, "nine ten", 0, true);
        t.setLengthLimit(10);
        t.setCursor(1, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "one three");
        TS_ASSERT_EQUALS(t.getLineText(1), "four five");
        TS_ASSERT_EQUALS(t.getLineText(2), "six sevn");
        TS_ASSERT_EQUALS(t.getLineText(3), "eight");
        TS_ASSERT_EQUALS(t.getLineText(4), "nine ten");
    }

    // Join lines, with re-wrap (2)
    {
        util::editor::Editor t;
        t.setLine(0, "one two", 0, true);
        t.setLine(1, "three-four", 0, true);
        t.setLine(2, "five six-", 0, true);
        t.setLine(3, "sevn eight", 0, false);
        t.setLine(4, "nine ten", 0, true);
        t.setLengthLimit(10);
        t.setCursor(1, 0);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "one three-");
        TS_ASSERT_EQUALS(t.getLineText(1), "four five");
        TS_ASSERT_EQUALS(t.getLineText(2), "six-sevn");
        TS_ASSERT_EQUALS(t.getLineText(3), "eight");
        TS_ASSERT_EQUALS(t.getLineText(4), "nine ten");
    }

    // Join lines, limit
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(1, 2);
        t.setUserLineLimit(1, 10);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Lorem ipsum dolor.");
        TS_ASSERT_EQUALS(t.getLineText(1), "sit amet.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
    }
}

/** Test cDeleteWordForward command. */
void
TestUtilEditorEditor::testCommandDeleteWordForward()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(1, 4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
        TS_ASSERT_EQUALS(t.getLineText(1), "  si amet.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
    }

    // End of line
    {
        util::editor::Editor t;
        t.setLine(0, "Lorem ipsum dolor.");
        t.setLine(1, "  sit amet.");
        t.setCursor(0, 20);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Lorem ipsum dolor.   amet.");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 20U);
    }

    // Joining lines, with wrap
    {
        util::editor::Editor t;
        //            123456789012345678901234567890
        t.setLine(0, "Duis sem velit,");
        t.setLine(1, "et, fermentum auctor, rhoncus ut.");
        t.setCursor(0, 16);
        t.setLengthLimit(30);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
        TS_ASSERT_EQUALS(t.getLineText(0), "Duis sem velit, , fermentum");
        TS_ASSERT_EQUALS(t.getLineText(1), "auctor, rhoncus ut.");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 16U);
    }
}

/** Test cTransposeCharacters command. */
void
TestUtilEditorEditor::testCommandTransposeCharacters()
{
    util::editor::Editor t;
    t.setLine(0, "transpose");
    t.setCursor(0, 5);
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cTransposeCharacters), true);
    TS_ASSERT_EQUALS(t.getLineText(0), "tranpsose");
    TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
    TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
}

/** Test cToggleInsert command. */
void
TestUtilEditorEditor::testCommandToggleInsert()
{
    util::editor::Editor t;
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cToggleInsert), false);
}

/** Test cToggleWrap command. */
void
TestUtilEditorEditor::testCommandToggleWrap()
{
    util::editor::Editor t;
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cToggleWrap), false);
}

/** Test cInsertTab command. */
void
TestUtilEditorEditor::testCommandInsertTab()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "id#   name");
        t.setLine(1, "35x");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
        TS_ASSERT_EQUALS(t.getLineText(1), "35    x");
    }

    // Overwrite
    {
        util::editor::Editor t;
        t.setLine(0, "id#   name");
        t.setLine(1, "35xxxyyzz");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(ed::Overwrite), ed::cInsertTab), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
        TS_ASSERT_EQUALS(t.getLineText(1), "35    yzz");
    }

    // No field name on previous line
    {
        util::editor::Editor t;
        t.setLine(0, "id#");
        t.setLine(1, "35");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 8U);
    }

    // No previous line
    {
        util::editor::Editor t;
        t.setLine(0, "35");
        t.setCursor(0, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 8U);
    }

    // Protected
    {
        util::editor::Editor t;
        t.setLine(0, "field: value", 6, true);
        t.setCursor(0, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
    }
}

/** Test cInsertNewline command. */
void
TestUtilEditorEditor::testCommandInsertNewline()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "onetwo");
        t.setCursor(0, 3);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "two");
    }

    // After end
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getLineText(2), "two");
    }

    // Two protected lines
    {
        util::editor::Editor t;
        t.setLine(0, "one", 1, false);
        t.setLine(1, "two", 1, false);
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 7U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "two");
    }
}

/** Test cInsertNewlineAbove command. */
void
TestUtilEditorEditor::testCommandInsertNewlineAbove()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setCursor(1, 2);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getLineText(2), "two");
    }

    // Line limit
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setLine(2, "three");
        t.setLine(3, "four");
        t.setCursor(1, 2);
        t.setLineLimit(4);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getLineText(2), "two");
        TS_ASSERT_EQUALS(t.getLineText(3), "three");
        TS_ASSERT_EQUALS(t.getLineText(4), "");
    }

    // Two protected lines
    {
        util::editor::Editor t;
        t.setLine(0, "one", 1, false);
        t.setLine(1, "two", 1, false);
        t.setCursor(0, 7);
        TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 7U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "two");
    }
}

/** Test cNull command. */
void
TestUtilEditorEditor::testCommandNull()
{
    util::editor::Editor t;
    TS_ASSERT_EQUALS(t.handleCommand(ed::Flags_t(), ed::cNull), true);
}

/** Test handleInsert(). */
void
TestUtilEditorEditor::testHandleInsert()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.handleInsert(ed::Flags_t(), "more ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 10U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more text");
    }

    // Overwrite
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.handleInsert(ed::Flags_t(ed::Overwrite), "n");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 6U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some next");
    }

    // Multi-line
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.handleInsert(ed::Flags_t(), "more\nnew ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "new text");
    }

    // Length limit
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(), "more ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 8U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some mortext");
    }

    // Length limit + wrap
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "more ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "text");
    }

    // Length limit + wrap (2)
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "more new ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "new text");
    }

    // Length limit + wrap (3)
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 0);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "insert ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 7U);
        TS_ASSERT_EQUALS(t.getLineText(0), "insert some");
        TS_ASSERT_EQUALS(t.getLineText(1), "text");
    }

    // Length limit + wrap (4)
    {
        util::editor::Editor t;
        t.setLine(0, "some-text");
        t.setCursor(0, 0);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "insert ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 7U);
        TS_ASSERT_EQUALS(t.getLineText(0), "insert some-");
        TS_ASSERT_EQUALS(t.getLineText(1), "text");
    }

    // Length limit + wrap, unicode
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(10);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "\xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4 \xC2\xA5\xC2\xA6\xC2\xA7 ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some \xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4");
        TS_ASSERT_EQUALS(t.getLineText(1), "\xC2\xA5\xC2\xA6\xC2\xA7 text");
    }

    // Long insert, length limit, wrap
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "more new exciting really long new ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "new exciting");
        TS_ASSERT_EQUALS(t.getLineText(2), "really long");
        TS_ASSERT_EQUALS(t.getLineText(3), "new text");
    }

    // Long insert, piecewise, wrap (exercises continuations)
    {
        util::editor::Editor t;
        t.setLine(0, "some text !");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "more ");
        t.handleInsert(ed::Flags_t(ed::WordWrap), "new");
        t.handleInsert(ed::Flags_t(ed::WordWrap), " ");
        t.handleInsert(ed::Flags_t(ed::WordWrap), "exciting ");
        t.handleInsert(ed::Flags_t(ed::WordWrap), "really ");
        t.handleInsert(ed::Flags_t(ed::WordWrap), "long ");
        t.handleInsert(ed::Flags_t(ed::WordWrap), "new ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "new exciting");
        TS_ASSERT_EQUALS(t.getLineText(2), "really long");
        TS_ASSERT_EQUALS(t.getLineText(3), "new text !");
    }

    // Long insert, no breakpoint, wrap
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setLine(1, "x");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "morenewexcitingreallylongnew ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 3U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 5U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some");
        TS_ASSERT_EQUALS(t.getLineText(1), "morenewexcit");
        TS_ASSERT_EQUALS(t.getLineText(2), "ingreallylon");
        TS_ASSERT_EQUALS(t.getLineText(3), "gnew text");
        TS_ASSERT_EQUALS(t.getLineText(4), "x");
    }

    // Long insert, length limit, wrap, line limit
    {
        util::editor::Editor t;
        t.setLine(0, "some text");
        t.setCursor(0, 5);
        t.setLengthLimit(12);
        t.setUserLineLimit(0, 2);
        t.handleInsert(ed::Flags_t(ed::WordWrap), "more new exciting really long new ");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 2U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 11U);
        TS_ASSERT_EQUALS(t.getLineText(0), "some more");
        TS_ASSERT_EQUALS(t.getLineText(1), "new exciting");
        TS_ASSERT_EQUALS(t.getLineText(2), "really long");
        TS_ASSERT_EQUALS(t.getLineText(3), "new text");
    }

    // Newline in marked fields
    {
        util::editor::Editor t;
        t.setLine(0, "From: ", 6, false);
        t.setLine(1, "To: ", 4, false);
        t.setCursor(0, 6);
        t.handleInsert(ed::Flags_t(), "one\ntwo");
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 13U);
        TS_ASSERT_EQUALS(t.getLineText(0), "From: one two");
    }
}

/** Test insertLine(). */
void
TestUtilEditorEditor::testInsertLine()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setCursor(1, 2);
        t.insertLine(1, 3);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 4U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 2U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "");
        TS_ASSERT_EQUALS(t.getLineText(2), "");
        TS_ASSERT_EQUALS(t.getLineText(3), "");
        TS_ASSERT_EQUALS(t.getLineText(4), "two");
        TS_ASSERT_EQUALS(t.getNumLines(), 5U);
    }

    // After end
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.insertLine(5, 3);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 0U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 0U);
        TS_ASSERT_EQUALS(t.getNumLines(), 8U);
    }
}

/** Test deleteLine(). */
void
TestUtilEditorEditor::testDeleteLine()
{
    // Normal
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setLine(2, "three");
        t.setLine(3, "four");
        t.setCursor(3, 1);
        t.deleteLine(1, 2);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 1U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getLineText(1), "four");
        TS_ASSERT_EQUALS(t.getNumLines(), 2U);
    }

    // Delete over end
    {
        util::editor::Editor t;
        t.setLine(0, "one");
        t.setLine(1, "two");
        t.setLine(2, "three");
        t.setLine(3, "four");
        t.setCursor(3, 4);
        t.deleteLine(1, 10);
        TS_ASSERT_EQUALS(t.getCurrentLine(), 1U);
        TS_ASSERT_EQUALS(t.getCurrentColumn(), 4U);
        TS_ASSERT_EQUALS(t.getLineText(0), "one");
        TS_ASSERT_EQUALS(t.getNumLines(), 1U);
    }
}

