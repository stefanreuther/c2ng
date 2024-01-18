/**
  *  \file test/util/editor/editortest.cpp
  *  \brief Test for util::editor::Editor
  */

#include "util/editor/editor.hpp"
#include "afl/test/testrunner.hpp"

namespace ed = util::editor;

/** Test basic configuration.
    A: call setCursor(), setLineLimit(), setLengthLimit().
    E: verify getters */
AFL_TEST("util.editor.Editor:config", a)
{
    util::editor::Editor t;
    a.checkEqual("01. getCurrentLine",   t.getCurrentLine(), 0U);
    a.checkEqual("02. getCurrentColumn", t.getCurrentColumn(), 0U);
    a.check     ("03. getLineLimit",     t.getLineLimit() > 1000U);
    a.check     ("04. getLengthLimit",   t.getLengthLimit() > 1000U);

    t.setCursor(7, 9);
    t.setLineLimit(12);
    t.setLengthLimit(32);

    a.checkEqual("11. getCurrentLine",   t.getCurrentLine(), 7U);
    a.checkEqual("12. getCurrentColumn", t.getCurrentColumn(), 9U);
    a.checkEqual("13. getLineLimit",     t.getLineLimit(), 12U);
    a.checkEqual("14. getLengthLimit",   t.getLengthLimit(), 32U);
}

/** Test basic text handling.
    A: call setLine()
    E: verify getLine(), getRange() */
AFL_TEST("util.editor.Editor:text", a)
{
    util::editor::Editor t;
    t.setLine(2, "hello");
    t.setLine(3, "world");

    a.checkEqual("01", t.getLineText(0), "");
    a.checkEqual("02", t.getLineText(1), "");
    a.checkEqual("03", t.getLineText(2), "hello");
    a.checkEqual("04", t.getLineText(3), "world");
    a.checkEqual("05", t.getLineText(4), "");

    a.checkEqual("11", t.getRange(0, 0, 7, 0), "\n\nhello\nworld\n\n\n\n");
    a.checkEqual("12", t.getRange(0, 10, 0, 20), "");
    a.checkEqual("13", t.getRange(2, 1, 2, 3), "el");
    a.checkEqual("14", t.getRange(2, 1, 3, 0), "ello\n");
    a.checkEqual("15", t.getRange(2, 1, 3, 2), "ello\nwo");
    a.checkEqual("16", t.getRange(2, 10, 3, 0), "\n");

    // Invalid
    a.checkEqual("21", t.getRange(2, 3, 2, 1), "");
    a.checkEqual("22", t.getRange(2, 3, 1, 0), "");
}

/*
 *  Test cMoveLineUp command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineUp:normal", a)
{
    util::editor::Editor t;
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 9U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Normal, AllowCursorAfterEnd
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineUp:AllowCursorAfterEnd", a)
{
    util::editor::Editor t;
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveLineUp), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 9U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

// At beginning
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineUp:at-beginning", a)
{
    util::editor::Editor t;
    t.setCursor(0, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

// With restriction
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineUp:restriction", a)
{
    util::editor::Editor t;
    t.setUserLineLimit(5, 10);
    t.setCursor(5, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineUp), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 5U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

/*
 *  Test cMoveLineDown command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineDown:normal", a)
{
    util::editor::Editor t;
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 11U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Normal, AllowCursorAfterEnd
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineDown:AllowCursorAfterEnd", a)
{
    util::editor::Editor t;
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveLineDown), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 11U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

// At end
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineDown:at-end", a)
{
    util::editor::Editor t;
    t.setLineLimit(10);
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

// With restriction
AFL_TEST("util.editor.Editor:handleCommand:cMoveLineDown:restriction", a)
{
    util::editor::Editor t;
    t.setUserLineLimit(5, 10);
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveLineDown), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

/*
 *  Test cMoveCharacterLeft command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cMoveCharacterLeft:normal", a)
{
    util::editor::Editor t;
    t.setLine(10, "123456789");
    t.setCursor(10, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveCharacterLeft), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 2U);
}

// At beginning
AFL_TEST("util.editor.Editor:handleCommand:cMoveCharacterLeft:at-beginning", a)
{
    util::editor::Editor t;
    t.setLine(10, "123456789");
    t.setCursor(10, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveCharacterLeft), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

/*
 *  Test cMoveCharacterRight command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cMoveCharacterRight:normal", a)
{
    util::editor::Editor t;
    t.setLine(10, "123456789");
    t.setCursor(10, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveCharacterRight), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
}

// At end
AFL_TEST("util.editor.Editor:handleCommand:cMoveCharacterRight:at-end", a)
{
    util::editor::Editor t;
    t.setLine(10, "12345");
    t.setLengthLimit(5);
    t.setCursor(10, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveCharacterRight), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 10U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 5U);
}


/** Test cMoveWordLeft command. */

// Sequence
AFL_TEST("util.editor.Editor:handleCommand:cMoveWordLeft:sequence", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");

    // Start at "a<m>et".
    t.setCursor(1, 7);

    // Go to "<a>met"
    a.checkEqual("handleCommand 1", t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
    a.checkEqual("getCurrentLine 1", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn 1", t.getCurrentColumn(), 6U);

    // Go to "<s>it"
    a.checkEqual("handleCommand 2", t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
    a.checkEqual("getCurrentLine 2", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn 2", t.getCurrentColumn(), 2U);

    // Go to beginning of line.
    a.checkEqual("handleCommand 3", t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
    a.checkEqual("getCurrentLine 3", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn 3", t.getCurrentColumn(), 0U);

    // Go to "<d>olor".
    a.checkEqual("handleCommand 4", t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
    a.checkEqual("getCurrentLine 4", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn 4", t.getCurrentColumn(), 12U);
}

// With limit
AFL_TEST("util.editor.Editor:handleCommand:cMoveWordLeft:limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "sit amet.");
    t.setUserLineLimit(1, 10);
    t.setCursor(1, 0);

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveWordLeft), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

/*
 *  Test cMoveWordRight command.
 */

// Sequence
AFL_TEST("util.editor.Editor:handleCommand:cMoveWordRight:sequence", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");

    // Start at "i<p>sum".
    t.setCursor(0, 7);

    // Go end of "ipsum"
    a.checkEqual("handleCommand 1", t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
    a.checkEqual("getCurrentLine 1", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn 1", t.getCurrentColumn(), 11U);

    // Go end of "dolor"
    a.checkEqual("handleCommand 2", t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
    a.checkEqual("getCurrentLine 2", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn 2", t.getCurrentColumn(), 17U);

    // Go end of line
    a.checkEqual("handleCommand 3", t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
    a.checkEqual("getCurrentLine 3", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn 3", t.getCurrentColumn(), 18U);

    // Go to next line
    a.checkEqual("handleCommand 4", t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
    a.checkEqual("getCurrentLine 4", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn 4", t.getCurrentColumn(), 5U);
}

// Limit
AFL_TEST("util.editor.Editor:handleCommand:cMoveWordRight:limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setUserLineLimit(0, 0);
    t.setCursor(0, 18);

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveWordRight), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 18U);
}

/*
 *  Test cMoveBeginningOfLine command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cMoveBeginningOfLine", a)
{
    util::editor::Editor t;
    t.setLine(3, "Subject: hi.", 9, false);
    t.setCursor(3, 11);

    // Go to beginning of editable
    a.checkEqual("handleCommand 1", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfLine), true);
    a.checkEqual("getCurrentLine 1", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn 1", t.getCurrentColumn(), 9U);

    // Go to beginning of line
    a.checkEqual("handleCommand 2", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfLine), true);
    a.checkEqual("getCurrentLine 2", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn 2", t.getCurrentColumn(), 0U);
}

/*
 *  Test cMoveEndOfLine command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cMoveEndOfLine", a)
{
    util::editor::Editor t;
    t.setLine(3, "Subject: hi.", 9, false);
    t.setCursor(3, 11);

    // Go to end
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveEndOfLine), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 12U);
}

/*
 *  Test cMoveBeginningOfDocument command.
 */

// Empty
AFL_TEST("util.editor.Editor:handleCommand:cMoveBeginningOfDocument:empty", a)
{
    util::editor::Editor t;
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Populated
AFL_TEST("util.editor.Editor:handleCommand:cMoveBeginningOfDocument:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "FROM: me", 1000, false);
    t.setLine(1, "TO: them", 1000, false);
    t.setLine(2, "Subject: hi.", 9, false);
    t.setLine(3, "hi");
    t.setLine(4, "there");

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 2U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 9U);
}

// Populated, Unicode
AFL_TEST("util.editor.Editor:handleCommand:cMoveBeginningOfDocument:unicode", a)
{
    util::editor::Editor t;
    t.setLine(0, "\xC2\xA1\xC2\xA2\xC2\xA3", 4, false);
    t.setLine(1, "hi");
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Line limit
AFL_TEST("util.editor.Editor:handleCommand:cMoveBeginningOfDocument:line-limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "a", 0, false);
    t.setLine(1, "b", 1000, false);
    t.setLine(2, "c", 1000, false);
    t.setLine(3, "hi");
    t.setLine(4, "there");
    t.setUserLineLimit(2, 100);

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveBeginningOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

/*
 *  Test cMoveEndOfDocument command.
 */


// Empty
AFL_TEST("util.editor.Editor:handleCommand:cMoveEndOfDocument:empty", a)
{
    util::editor::Editor t;
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Populated
AFL_TEST("util.editor.Editor:handleCommand:cMoveEndOfDocument:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "FROM: me", 1000, false);
    t.setLine(1, "TO: them", 1000, false);
    t.setLine(2, "Subject: hi.", 9, false);
    t.setLine(3, "hi");
    t.setLine(4, "there");

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 4U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 5U);
}

// Limit
AFL_TEST("util.editor.Editor:handleCommand:cMoveEndOfDocument:limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "a");
    t.setLine(1, "b");
    t.setLine(2, "c");
    t.setLine(3, "d");
    t.setLine(4, "e");
    t.setUserLineLimit(0, 3);

    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cMoveEndOfDocument), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 1U);
}

/*
 *  Test cDeleteCharacter command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hell");
    a.checkEqual("getLineText(1)", t.getLineText(1), "there");
}

// Protected
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "hi: there", 4, false);
    t.setCursor(0, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hi: there");
}

// Joining lines
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:joining-lines", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hellothere");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
}

// Joining lines, cursor after end
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:joining-lines:after-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello  there");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
}

// Joining lines, with wrap
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:joining-lines:wrap", a)
{
    util::editor::Editor t;
    //            123456789012345678901234567890
    t.setLine(0, "Duis sem velit, ultrices ");
    t.setLine(1, "et, fermentum auctor, rhoncus ut.");
    t.setCursor(0, 25);
    t.setLengthLimit(30);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Duis sem velit, ultrices et,");
    a.checkEqual("getLineText(1)", t.getLineText(1), "fermentum auctor, rhoncus ut.");
}

// Joining lines, making long word
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:joining-lines:long-word", a)
{
    util::editor::Editor t;
    t.setLine(0, "One two");
    t.setLine(1, "three four");
    t.setCursor(0, 7);
    t.setLengthLimit(9);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "One");
    a.checkEqual("getLineText(1)", t.getLineText(1), "twothree");
    a.checkEqual("getLineText(2)", t.getLineText(2), "four");
}

// Next line protected
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacter:next-line-protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there", 1, true);
    t.setCursor(0, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacter), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "there");
}

/*
 *  Test cDeleteCharacterBackward command.
 */


// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(1, 4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "thee");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
}

// With overwrite - FIXME: not implemented in single-line handler
// {
//     util::editor::Editor t;
//     t.setLine(0, "hello");
//     t.setLine(1, "there");
//     t.setCursor(1, 4);
//     a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(ed::Overwrite), ed::cDeleteCharacterBackward), true);
//     a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
//     a.checkEqual("getLineText(1)", t.getLineText(1), "the e");
//     a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
//     a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 3U);
// }

// Protected
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "hi: there", 4, false);
    t.setCursor(0, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hi: there");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 1U);
}

// Cursor after end
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:after-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "there");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
}

// Joining lines
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:joining-lines", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(1, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hellothere");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 5U);
}

// Joining lines, with wrap
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:joining-lines:wrap", a)
{
    util::editor::Editor t;
    //            123456789012345678901234567890
    t.setLine(0, "Duis sem velit, ultrices ");
    t.setLine(1, "et, fermentum auctor, rhoncus ut.");
    t.setCursor(1, 0);
    t.setLengthLimit(30);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Duis sem velit, ultrices et,");
    a.checkEqual("getLineText(1)", t.getLineText(1), "fermentum auctor, rhoncus ut.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 25U);
}

// Joining lines, limit
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:joining-lines:limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(1, 0);
    t.setUserLineLimit(1, 10);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "there");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Previous line protected
AFL_TEST("util.editor.Editor:handleCommand:cDeleteCharacterBackward:previous-line-protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello", 1000, true);
    t.setLine(1, "there");
    t.setCursor(1, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteCharacterBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "there");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

/*
 *  Test cDeleteLine command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteLine:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setLine(2, "three");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "three");
    a.checkEqual("getLineText(2)", t.getLineText(2), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Protected line
AFL_TEST("util.editor.Editor:handleCommand:cDeleteLine:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two: half", 4, false);
    t.setLine(2, "three");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "two:");
    a.checkEqual("getLineText(2)", t.getLineText(2), "three");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U); // end of protected area
}

/*
 *  Test cDeleteEndOfLine command.
 */


// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteEndOfLine:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(1, 4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello");
    a.checkEqual("getLineText(1)", t.getLineText(1), "ther");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
}

// Protected
AFL_TEST("util.editor.Editor:handleCommand:cDeleteEndOfLine:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "hi: there", 4, false);
    t.setCursor(0, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hi: ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
}

// Cursor after end
AFL_TEST("util.editor.Editor:handleCommand:cDeleteEndOfLine:after-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hello  there");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 7U);
}

// Joining lines
AFL_TEST("util.editor.Editor:handleCommand:cDeleteEndOfLine:joining-lines", a)
{
    util::editor::Editor t;
    t.setLine(0, "hello");
    t.setLine(1, "there");
    t.setCursor(0, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteEndOfLine), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "hellothere");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 5U);
}

/*
 *  Test cDeleteWordBackward command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(1, 4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(1)", t.getLineText(1), "  t amet.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 2U);
}

// Beginning
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:beginning", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(1)", t.getLineText(1), "sit amet.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

// Join lines
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:joining-lines", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(1, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Lorem ipsum   sit amet.");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 12U);
}

// Joining lines, with wrap
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:joining-lines:wrap", a)
{
    util::editor::Editor t;
    //            123456789012345678901234567890
    t.setLine(0, "Duis sem velit, ultrices ");
    t.setLine(1, "et, fermentum auctor, rhoncus ut.");
    t.setCursor(1, 0);
    t.setLengthLimit(30);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Duis sem velit, et, fermentum");
    a.checkEqual("getLineText(1)", t.getLineText(1), "auctor, rhoncus ut.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 16U);
}

// Join lines, with re-wrap
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:joining-lines:re-wrap", a)
{
    util::editor::Editor t;
    t.setLine(0, "one two", 0, true);
    t.setLine(1, "three four", 0, true);
    t.setLine(2, "five six", 0, true);
    t.setLine(3, "sevn eight", 0, false);
    t.setLine(4, "nine ten", 0, true);
    t.setLengthLimit(10);
    t.setCursor(1, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one three");
    a.checkEqual("getLineText(1)", t.getLineText(1), "four five");
    a.checkEqual("getLineText(2)", t.getLineText(2), "six sevn");
    a.checkEqual("getLineText(3)", t.getLineText(3), "eight");
    a.checkEqual("getLineText(4)", t.getLineText(4), "nine ten");
}

// Join lines, with re-wrap (2)
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:joining-lines:re-wrap:hyphen", a)
{
    util::editor::Editor t;
    t.setLine(0, "one two", 0, true);
    t.setLine(1, "three-four", 0, true);
    t.setLine(2, "five six-", 0, true);
    t.setLine(3, "sevn eight", 0, false);
    t.setLine(4, "nine ten", 0, true);
    t.setLengthLimit(10);
    t.setCursor(1, 0);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one three-");
    a.checkEqual("getLineText(1)", t.getLineText(1), "four five");
    a.checkEqual("getLineText(2)", t.getLineText(2), "six-sevn");
    a.checkEqual("getLineText(3)", t.getLineText(3), "eight");
    a.checkEqual("getLineText(4)", t.getLineText(4), "nine ten");
}

// Join lines, limit
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordBackward:joining-lines:limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(1, 2);
    t.setUserLineLimit(1, 10);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordBackward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Lorem ipsum dolor.");
    a.checkEqual("getLineText(1)", t.getLineText(1), "sit amet.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
}

/*
 *  Test cDeleteWordForward command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordForward:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(1, 4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
    a.checkEqual("getLineText(1)", t.getLineText(1), "  si amet.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
}

// End of line
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordForward:end-of-line", a)
{
    util::editor::Editor t;
    t.setLine(0, "Lorem ipsum dolor.");
    t.setLine(1, "  sit amet.");
    t.setCursor(0, 20);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Lorem ipsum dolor.   amet.");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 20U);
}

// Joining lines, with wrap
AFL_TEST("util.editor.Editor:handleCommand:cDeleteWordForward:joining-lines", a)
{
    util::editor::Editor t;
    //            123456789012345678901234567890
    t.setLine(0, "Duis sem velit,");
    t.setLine(1, "et, fermentum auctor, rhoncus ut.");
    t.setCursor(0, 16);
    t.setLengthLimit(30);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cDeleteWordForward), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "Duis sem velit, , fermentum");
    a.checkEqual("getLineText(1)", t.getLineText(1), "auctor, rhoncus ut.");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 16U);
}

/*
 *  Test cTransposeCharacters command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cTransposeCharacters", a)
{
    util::editor::Editor t;
    t.setLine(0, "transpose");
    t.setCursor(0, 5);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cTransposeCharacters), true);
    a.checkEqual("getLineText(0)", t.getLineText(0), "tranpsose");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
}

/*
 *  Test cToggleInsert command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cToggleInsert", a)
{
    util::editor::Editor t;
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cToggleInsert), false);
}

/*
 *  Test cToggleWrap command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cToggleWrap", a)
{
    util::editor::Editor t;
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cToggleWrap), false);
}

/*
 *  Test cInsertTab command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cInsertTab:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "id#   name");
    t.setLine(1, "35x");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
    a.checkEqual("getLineText(1)", t.getLineText(1), "35    x");
}

// Overwrite
AFL_TEST("util.editor.Editor:handleCommand:cInsertTab:overwrite", a)
{
    util::editor::Editor t;
    t.setLine(0, "id#   name");
    t.setLine(1, "35xxxyyzz");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(ed::Overwrite), ed::cInsertTab), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
    a.checkEqual("getLineText(1)", t.getLineText(1), "35    yzz");
}

// No field name on previous line
AFL_TEST("util.editor.Editor:handleCommand:cInsertTab:no-field", a)
{
    util::editor::Editor t;
    t.setLine(0, "id#");
    t.setLine(1, "35");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 8U);
}

// No previous line
AFL_TEST("util.editor.Editor:handleCommand:cInsertTab:no-previous-line", a)
{
    util::editor::Editor t;
    t.setLine(0, "35");
    t.setCursor(0, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 8U);
}

// Protected
AFL_TEST("util.editor.Editor:handleCommand:cInsertTab: protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "field: value", 6, true);
    t.setCursor(0, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertTab), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
}

/*
 *  Test cInsertNewline command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewline:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "onetwo");
    t.setCursor(0, 3);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "two");
}

// After end
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewline:after-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getLineText(2)", t.getLineText(2), "two");
}

// Two protected lines
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewline:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "one", 1, false);
    t.setLine(1, "two", 1, false);
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewline), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 7U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "two");
}

/*
 *  Test cInsertNewlineAbove command.
 */

// Normal
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewlineAbove:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setCursor(1, 2);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 2U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getLineText(2)", t.getLineText(2), "two");
}

// Line limit
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewlineAbove:line-limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setLine(2, "three");
    t.setLine(3, "four");
    t.setCursor(1, 2);
    t.setLineLimit(4);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 2U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getLineText(2)", t.getLineText(2), "two");
    a.checkEqual("getLineText(3)", t.getLineText(3), "three");
    a.checkEqual("getLineText(4)", t.getLineText(4), "");
}

// Two protected lines
AFL_TEST("util.editor.Editor:handleCommand:cInsertNewlineAbove:protected", a)
{
    util::editor::Editor t;
    t.setLine(0, "one", 1, false);
    t.setLine(1, "two", 1, false);
    t.setCursor(0, 7);
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cInsertNewlineAbove), true);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 7U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "two");
}

/*
 *  Test cNull command.
 */

AFL_TEST("util.editor.Editor:handleCommand:cNull", a)
{
    util::editor::Editor t;
    a.checkEqual("handleCommand", t.handleCommand(ed::Flags_t(), ed::cNull), true);
}

/*
 *  Test handleInsert().
 */

// Normal
AFL_TEST("util.editor.Editor:handleInsert:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.handleInsert(ed::Flags_t(), "more ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 10U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more text");
}

// Overwrite
AFL_TEST("util.editor.Editor:handleInsert:overwrite", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.handleInsert(ed::Flags_t(ed::Overwrite), "n");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 6U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some next");
}

// Multi-line
AFL_TEST("util.editor.Editor:handleInsert:multiline", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.handleInsert(ed::Flags_t(), "more\nnew ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "new text");
}

// Length limit
AFL_TEST("util.editor.Editor:handleInsert:length-limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(), "more ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 8U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some mortext");
}

// Length limit + wrap
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "more ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "text");
}

// Length limit + wrap (2)
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:2", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "more new ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "new text");
}

// Length limit + wrap (3)
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:3", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 0);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "insert ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 7U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "insert some");
    a.checkEqual("getLineText(1)", t.getLineText(1), "text");
}

// Length limit + wrap (4)
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:hyphen", a)
{
    util::editor::Editor t;
    t.setLine(0, "some-text");
    t.setCursor(0, 0);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "insert ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 7U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "insert some-");
    a.checkEqual("getLineText(1)", t.getLineText(1), "text");
}

// Length limit + wrap, unicode
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:unicode", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(10);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "\xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4 \xC2\xA5\xC2\xA6\xC2\xA7 ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some \xC2\xA1\xC2\xA2\xC2\xA3\xC2\xA4");
    a.checkEqual("getLineText(1)", t.getLineText(1), "\xC2\xA5\xC2\xA6\xC2\xA7 text");
}

// Long insert, length limit, wrap
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:long-insert", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "more new exciting really long new ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "new exciting");
    a.checkEqual("getLineText(2)", t.getLineText(2), "really long");
    a.checkEqual("getLineText(3)", t.getLineText(3), "new text");
}

// Long insert, piecewise, wrap (exercises continuations)
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:piecewise", a)
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
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "new exciting");
    a.checkEqual("getLineText(2)", t.getLineText(2), "really long");
    a.checkEqual("getLineText(3)", t.getLineText(3), "new text !");
}

// Long insert, no breakpoint, wrap
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:unbreakable", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setLine(1, "x");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "morenewexcitingreallylongnew ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 3U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 5U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some");
    a.checkEqual("getLineText(1)", t.getLineText(1), "morenewexcit");
    a.checkEqual("getLineText(2)", t.getLineText(2), "ingreallylon");
    a.checkEqual("getLineText(3)", t.getLineText(3), "gnew text");
    a.checkEqual("getLineText(4)", t.getLineText(4), "x");
}

// Long insert, length limit, wrap, line limit
AFL_TEST("util.editor.Editor:handleInsert:length-limit-wrap:line-limit", a)
{
    util::editor::Editor t;
    t.setLine(0, "some text");
    t.setCursor(0, 5);
    t.setLengthLimit(12);
    t.setUserLineLimit(0, 2);
    t.handleInsert(ed::Flags_t(ed::WordWrap), "more new exciting really long new ");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 2U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 11U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "some more");
    a.checkEqual("getLineText(1)", t.getLineText(1), "new exciting");
    a.checkEqual("getLineText(2)", t.getLineText(2), "really long");
    a.checkEqual("getLineText(3)", t.getLineText(3), "new text");
}

// Newline in marked fields
AFL_TEST("util.editor.Editor:handleInsert:newline-in-field", a)
{
    util::editor::Editor t;
    t.setLine(0, "From: ", 6, false);
    t.setLine(1, "To: ", 4, false);
    t.setCursor(0, 6);
    t.handleInsert(ed::Flags_t(), "one\ntwo");
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 13U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "From: one two");
}

/*
 *  Test insertLine().
 */

// Normal
AFL_TEST("util.editor.Editor:insertLine:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setCursor(1, 2);
    t.insertLine(1, 3);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 4U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 2U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "");
    a.checkEqual("getLineText(2)", t.getLineText(2), "");
    a.checkEqual("getLineText(3)", t.getLineText(3), "");
    a.checkEqual("getLineText(4)", t.getLineText(4), "two");
    a.checkEqual("getNumLines", t.getNumLines(), 5U);
}

// After end
AFL_TEST("util.editor.Editor:insertLine:after-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.insertLine(5, 3);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 0U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 0U);
    a.checkEqual("getNumLines", t.getNumLines(), 8U);
}

/*
 *  Test deleteLine().
 */

// Normal
AFL_TEST("util.editor.Editor:deleteLine:normal", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setLine(2, "three");
    t.setLine(3, "four");
    t.setCursor(3, 1);
    t.deleteLine(1, 2);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 1U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getLineText(1)", t.getLineText(1), "four");
    a.checkEqual("getNumLines", t.getNumLines(), 2U);
}

// Delete over end
AFL_TEST("util.editor.Editor:deleteLine:over-end", a)
{
    util::editor::Editor t;
    t.setLine(0, "one");
    t.setLine(1, "two");
    t.setLine(2, "three");
    t.setLine(3, "four");
    t.setCursor(3, 4);
    t.deleteLine(1, 10);
    a.checkEqual("getCurrentLine", t.getCurrentLine(), 1U);
    a.checkEqual("getCurrentColumn", t.getCurrentColumn(), 4U);
    a.checkEqual("getLineText(0)", t.getLineText(0), "one");
    a.checkEqual("getNumLines", t.getNumLines(), 1U);
}
