/**
  *  \file test/util/editor/commandtest.cpp
  *  \brief Test for util::editor::Command
  */

#include "util/editor/command.hpp"
#include "afl/test/testrunner.hpp"

namespace ed = util::editor;

/*
 *  Tests are performed using Unicode characters to make sure that the byte/char mapping is correct.
 */
#define C1 "\xC2\xA1"
#define C2 "\xC2\xA2"
#define C3 "\xC2\xA3"
#define C4 "\xC2\xA4"
#define C5 "\xC2\xA5"
#define C6 "\xC2\xA6"


AFL_TEST("util.editor.Command:toString", a)
{
    // Coverage
    for (size_t i = 0; i < ed::NUM_COMMANDS; ++i) {
        a.checkNonNull("01", ed::toString(ed::Command(i)));
    }

    // Specimen
    a.checkEqual("11", ed::toString(ed::cDeleteCharacter), String_t("DeleteCharacter"));
}

AFL_TEST("util.editor.Command:lookupKey", a)
{
    ed::Command cmd;

    // Positive case
    a.checkEqual("01", ed::lookupKey(util::Key_Delete, cmd), true);
    a.checkEqual("02", cmd, ed::cDeleteCharacter);

    a.checkEqual("11", ed::lookupKey('t' + util::KeyMod_Ctrl, cmd), true);
    a.checkEqual("12", cmd, ed::cTransposeCharacters);

    // Negative case
    a.checkEqual("21", ed::lookupKey('t', cmd), false);
    a.checkEqual("22", ed::lookupKey(util::Key_F1, cmd), false);
}

// Multi-line commands
AFL_TEST("util.editor.Command:handleCommand:reject-multiline", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    a.checkEqual("01", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveLineUp,              String_t::npos), false);
    a.checkEqual("02", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveLineDown,            String_t::npos), false);
    a.checkEqual("03", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveBeginningOfDocument, String_t::npos), false);
    a.checkEqual("04", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveEndOfDocument,       String_t::npos), false);
    a.checkEqual("05", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cToggleInsert,            String_t::npos), false);
    a.checkEqual("06", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cToggleWrap,              String_t::npos), false);
    a.checkEqual("07", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertTab,               String_t::npos), false);
    a.checkEqual("08", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertNewline,           String_t::npos), false);
    a.checkEqual("09", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertNewlineAbove,      String_t::npos), false);
}

/*
 *  handleCommand
 */

// Move left
AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterLeft", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterLeft, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 0U);
}

AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterLeft:at-beginning", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(0);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterLeft, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 0U);
}

// Move right
AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterRight", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterRight, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterRight:at-end", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterRight, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterRight:after-end-allowed", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveCharacterRight, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 4U);
}

AFL_TEST("util.editor.Command:handleCommand:cMoveCharacterRight:after-end-limited", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(6);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveCharacterRight, 6), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 6U);
}

// Word left
AFL_TEST("util.editor.Command:handleCommand:cMoveWordLeft", a)
{
    String_t line(" " C1 C2 C3 " " C4 C5);
    size_t cursor(7);
    a.checkEqual("handleCommand 1", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
    a.checkEqual("cursor 1", cursor, 5U);
    a.checkEqual("handleCommand 2", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
    a.checkEqual("cursor 2", cursor, 1U);
    a.checkEqual("handleCommand 3", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
    a.checkEqual("cursor 3", cursor, 0U);
    a.checkEqual("line", line, " " C1 C2 C3 " " C4 C5);
}

// Word right
AFL_TEST("util.editor.Command:handleCommand:cMoveWordRight", a)
{
    String_t line(" " C1 C2 C3 " " C4 C5 ")");
    size_t cursor(0);
    a.checkEqual("handleCommand 1", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
    a.checkEqual("cursor 1", cursor, 4U);
    a.checkEqual("handleCommand 2", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
    a.checkEqual("cursor 2", cursor, 7U);
    a.checkEqual("handleCommand 3", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
    a.checkEqual("cursor 3", cursor, 8U);
    a.checkEqual("line", line, " " C1 C2 C3 " " C4 C5 ")");
}

// Beginning of line
AFL_TEST("util.editor.Command:handleCommand:cMoveBeginningOfLine", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(3);
    a.checkEqual("handleCommand 1", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveBeginningOfLine, String_t::npos), true);
    a.checkEqual("cursor 1", cursor, 1U);
    a.checkEqual("handleCommand 2", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveBeginningOfLine, String_t::npos), true);
    a.checkEqual("cursor 2", cursor, 0U);
    a.checkEqual("line", line, C1 C2 C3);
}

// End of line
AFL_TEST("util.editor.Command:handleCommand:cMoveEndOfLine", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(0);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveEndOfLine, String_t::npos), true);
    a.checkEqual("cursor", cursor, 3U);
    a.checkEqual("line", line, C1 C2 C3);
}

// Delete
AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacter:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteCharacter, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacter:TypeErase", a)
{
    // ...TypeErase
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::TypeErase), ed::cDeleteCharacter, String_t::npos), true);
    a.checkEqual("line", line, C1);
    a.checkEqual("cursor", cursor, 1U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacter:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacter, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C4);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacter:protected", a)
{
    // ...in protected range
    String_t line(C1 C2 C3 C4);
    size_t cursor(0);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacter, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 C4);
    a.checkEqual("cursor", cursor, 0U);
}

// Delete backward
AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacterBackward:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteCharacterBackward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 1U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacterBackward:TypeErase", a)
{
    // ...TypeErase
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::TypeErase), ed::cDeleteCharacterBackward, String_t::npos), true);
    a.checkEqual("line", line, C1);
    a.checkEqual("cursor", cursor, 1U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacterBackward:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacterBackward, String_t::npos), true);
    a.checkEqual("line", line, C1 C3 C4);
    a.checkEqual("cursor", cursor, 1U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteCharacterBackward:protected", a)
{
    // ...in protected range
    String_t line(C1 C2 C3 C4);
    size_t cursor(1);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteCharacterBackward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 C4);
    a.checkEqual("cursor", cursor, 0U);
}

// Delete line
AFL_TEST("util.editor.Command:handleCommand:cDeleteLine:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteLine, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteLine:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteLine, String_t::npos), true);
    a.checkEqual("line", line, C1);
    a.checkEqual("cursor", cursor, 1U);
}

// Delete end of line
AFL_TEST("util.editor.Command:handleCommand:cDeleteEndOfLine:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteEndOfLine, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteEndOfLine:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteEndOfLine, String_t::npos), true);
    a.checkEqual("line", line, C1 C2);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteEndOfLine:after-end", a)
{
    // ...beyond end
    String_t line(C1 C2 C3);
    size_t cursor(7);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteEndOfLine, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 3U);
}

// Delete word backward
AFL_TEST("util.editor.Command:handleCommand:cDeleteWordBackward:NonEditable", a)
{
    // ...NonEditable
    String_t line(" " C1 C2 C3 " " C4 C5 C6);
    size_t cursor(7);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 3, ed::Flags_t(ed::NonEditable), ed::cDeleteWordBackward, String_t::npos), true);
    a.checkEqual("line", line, " " C1 C2 C3 " " C4 C5 C6);
    a.checkEqual("cursor", cursor, 5U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordBackward:normal", a)
{
    // ...normal
    String_t line(" " C1 C2 C3 " " C4 C5 C6);
    size_t cursor(7);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
    a.checkEqual("line", line, " " C1 C2 C3 " " C6);
    a.checkEqual("cursor", cursor, 5U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordBackward:into-protected", a)
{
    // ...into protected
    String_t line(" " C1 C2 C3 " " C6);
    size_t cursor(5);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
    a.checkEqual("line", line, " " C1 C2 C6);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordBackward:protected", a)
{
    // ...in protected
    String_t line(" " C1 C2 C3 " " C6);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
    a.checkEqual("line", line, " " C1 C2 C3 " " C6);
    a.checkEqual("cursor", cursor, 1U);
}

// Delete word forward
AFL_TEST("util.editor.Command:handleCommand:cDeleteWordForward:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3 C4 " " C5 C6);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(ed::NonEditable), ed::cDeleteWordForward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 C4 " " C5 C6);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordForward:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3 C4 " " C5 C6);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 " " C5 C6);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordForward:at-word-end", a)
{
    // ...normal, too
    String_t line(C1 C2 C3 " " C5 C6);
    size_t cursor(3);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cDeleteWordForward:protected", a)
{
    // ...protected
    String_t line(C1 C2 C3 C4 " " C5 C6);
    size_t cursor(1);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 C4 " " C5 C6);
    a.checkEqual("cursor", cursor, 1U);
}

// Transpose
AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:NonEditable", a)
{
    // ...NonEditable
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::NonEditable), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3 C4);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:normal", a)
{
    // ...normal
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C1 C3 C2 C4);
    a.checkEqual("cursor", cursor, 3U);
}

AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:start", a)
{
    // ...start
    String_t line(C1 C2 C3 C4);
    size_t cursor(0);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C2 C1 C3 C4);
    a.checkEqual("cursor", cursor, 2U);
}

AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:end", a)
{
    // ...end
    String_t line(C1 C2 C3 C4);
    size_t cursor(4);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C4 C3);
    a.checkEqual("cursor", cursor, 4U);
}

AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:protected", a)
{
    // ...protection
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C4 C3);
    a.checkEqual("cursor", cursor, 4U);
}
AFL_TEST("util.editor.Command:handleCommand:cTransposeCharacters:too-short", a)
{
    // ...too short
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 2U);
}

// Null
AFL_TEST("util.editor.Command:handleCommand:cNull", a)
{
    // ...too short
    String_t line(C1 C2 C3);
    size_t cursor(2);
    a.checkEqual("handleCommand", ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cNull, String_t::npos), true);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 2U);
}

/*
 *  handleInsert
 */


// Not editable
AFL_TEST("util.editor.Command:handleInsert:NonEditable", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::NonEditable), C4, String_t::npos);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 1U);
}

// TypeErase, normal case
AFL_TEST("util.editor.Command:handleInsert:TypeErase", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::TypeErase), C4 C5, String_t::npos);
    a.checkEqual("line", line, C4 C5);
    a.checkEqual("cursor", cursor, 2U);
}

// TypeErase, with protected part
AFL_TEST("util.editor.Command:handleInsert:TypeErase:protected", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 1, ed::Flags_t(ed::TypeErase), C4 C5, String_t::npos);
    a.checkEqual("line", line, C1 C4 C5);
    a.checkEqual("cursor", cursor, 3U);
}

// TypeErase, with length limit and protected part
AFL_TEST("util.editor.Command:handleInsert:limited", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(0);
    ed::handleInsert(line, cursor, 2, ed::Flags_t(ed::TypeErase), C4 C5 C6, 5);
    a.checkEqual("line", line, C1 C2 C4 C5 C6);
    a.checkEqual("cursor", cursor, 5U);
}

// Normal, cursor in protected part (cannot insert here)
AFL_TEST("util.editor.Command:handleInsert:protected", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 2, ed::Flags_t(), C4 C5, String_t::npos);
    a.checkEqual("line", line, C1 C2 C3);
    a.checkEqual("cursor", cursor, 1U);
}

// Normal
AFL_TEST("util.editor.Command:handleInsert:normal", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 1, ed::Flags_t(), C4 C5, String_t::npos);
    a.checkEqual("line", line, C1 C4 C5 C2 C3);
    a.checkEqual("cursor", cursor, 3U);
}

// Cursor after end
AFL_TEST("util.editor.Command:handleInsert:after-end", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(5);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(), C4 C5, String_t::npos);
    a.checkEqual("line", line, C1 C2 C3 "  " C4 C5);
    a.checkEqual("cursor", cursor, 7U);
}

// Length limit
AFL_TEST("util.editor.Command:handleInsert:length-limit", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(2);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(), C4 C5 C6, 5);
    a.checkEqual("line", line, C1 C2 C4 C5 C3);
    a.checkEqual("cursor", cursor, 4U);
}

// Over limit
AFL_TEST("util.editor.Command:handleInsert:over-limit", a)
{
    String_t line(C1 C2 C3 C4);
    size_t cursor(2);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(), C5 C6, 3);
    a.checkEqual("line", line, C1 C2 C3 C4);
    a.checkEqual("cursor", cursor, 2U);
}

// Overwrite
AFL_TEST("util.editor.Command:handleInsert:Overwrite", a)
{
    String_t line(C1 C2 C3);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), C4 C5 C6, String_t::npos);
    a.checkEqual("line", line, C1 C4 C5 C6);
    a.checkEqual("cursor", cursor, 4U);
}

// Overwrite by shorter runes
AFL_TEST("util.editor.Command:handleInsert:Overwrite:shorter-runes", a)
{
    String_t line(C1 C2 C3 C4);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), "ab", String_t::npos);
    a.checkEqual("line", line, C1 "ab" C4);
    a.checkEqual("cursor", cursor, 3U);
}

// Overwrite by longer runes
AFL_TEST("util.editor.Command:handleInsert:Overwrite:longer-runes", a)
{
    String_t line(C1 "ab" C4);
    size_t cursor(1);
    ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), C5 C6, String_t::npos);
    a.checkEqual("line", line, C1 C5 C6 C4);
    a.checkEqual("cursor", cursor, 3U);
}
