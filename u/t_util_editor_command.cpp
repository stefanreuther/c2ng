/**
  *  \file u/t_util_editor_command.cpp
  *  \brief Test for util::editor::Command
  */

#include "util/editor/command.hpp"

#include "t_util_editor.hpp"

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


void
TestUtilEditorCommand::testToString()
{
    // Coverage
    for (size_t i = 0; i < ed::NUM_COMMANDS; ++i) {
        TS_ASSERT(ed::toString(ed::Command(i)) != 0);
    }

    // Specimen
    TS_ASSERT_EQUALS(ed::toString(ed::cDeleteCharacter), String_t("DeleteCharacter"));
}

void
TestUtilEditorCommand::testLookup()
{
    ed::Command cmd;

    // Positive case
    TS_ASSERT_EQUALS(ed::lookupKey(util::Key_Delete, cmd), true);
    TS_ASSERT_EQUALS(cmd, ed::cDeleteCharacter);

    TS_ASSERT_EQUALS(ed::lookupKey('t' + util::KeyMod_Ctrl, cmd), true);
    TS_ASSERT_EQUALS(cmd, ed::cTransposeCharacters);

    // Negative case
    TS_ASSERT_EQUALS(ed::lookupKey('t', cmd), false);
    TS_ASSERT_EQUALS(ed::lookupKey(util::Key_F1, cmd), false);
}

void
TestUtilEditorCommand::testHandleCommand()
{
    // Multi-line commands
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveLineUp,              String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveLineDown,            String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveBeginningOfDocument, String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveEndOfDocument,       String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cToggleInsert,            String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cToggleWrap,              String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertTab,               String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertNewline,           String_t::npos), false);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cInsertNewlineAbove,      String_t::npos), false);
    }

    // Move left
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterLeft, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 0U);
    }
    {
        String_t line(C1 C2 C3);
        size_t cursor(0);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterLeft, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 0U);
    }

    // Move right
    {
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterRight, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        String_t line(C1 C2 C3);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveCharacterRight, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        String_t line(C1 C2 C3);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveCharacterRight, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 4U);
    }
    {
        String_t line(C1 C2 C3);
        size_t cursor(6);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::AllowCursorAfterEnd), ed::cMoveCharacterRight, 6), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 6U);
    }

    // Word left
    {
        String_t line(" " C1 C2 C3 " " C4 C5);
        size_t cursor(7);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 5U);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 1U);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordLeft, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 0U);
        TS_ASSERT_EQUALS(line, " " C1 C2 C3 " " C4 C5);
    }

    // Word right
    {
        String_t line(" " C1 C2 C3 " " C4 C5 ")");
        size_t cursor(0);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 4U);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 7U);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cMoveWordRight, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 8U);
        TS_ASSERT_EQUALS(line, " " C1 C2 C3 " " C4 C5 ")");
    }

    // Beginning of line
    {
        String_t line(C1 C2 C3);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveBeginningOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 1U);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveBeginningOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 0U);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
    }

    // End of line
    {
        String_t line(C1 C2 C3);
        size_t cursor(0);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cMoveEndOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(cursor, 3U);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
    }

    // Delete
    {
        // ...NonEditable
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteCharacter, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...TypeErase
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::TypeErase), ed::cDeleteCharacter, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1);
        TS_ASSERT_EQUALS(cursor, 1U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacter, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C4);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...in protected range
        String_t line(C1 C2 C3 C4);
        size_t cursor(0);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacter, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4);
        TS_ASSERT_EQUALS(cursor, 0U);
    }

    // Delete backward
    {
        // ...NonEditable
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteCharacterBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 1U);
    }
    {
        // ...TypeErase
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::TypeErase), ed::cDeleteCharacterBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1);
        TS_ASSERT_EQUALS(cursor, 1U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteCharacterBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C3 C4);
        TS_ASSERT_EQUALS(cursor, 1U);
    }
    {
        // ...in protected range
        String_t line(C1 C2 C3 C4);
        size_t cursor(1);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteCharacterBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4);
        TS_ASSERT_EQUALS(cursor, 0U);
    }

    // Delete line
    {
        // ...NonEditable
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteLine, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteLine, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1);
        TS_ASSERT_EQUALS(cursor, 1U);
    }

    // Delete end of line
    {
        // ...NonEditable
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(ed::NonEditable), ed::cDeleteEndOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteEndOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...beyond end
        String_t line(C1 C2 C3);
        size_t cursor(7);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 1, ed::Flags_t(), ed::cDeleteEndOfLine, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 3U);
    }

    // Delete word backward
    {
        // ...NonEditable
        String_t line(" " C1 C2 C3 " " C4 C5 C6);
        size_t cursor(7);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 3, ed::Flags_t(ed::NonEditable), ed::cDeleteWordBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, " " C1 C2 C3 " " C4 C5 C6);
        TS_ASSERT_EQUALS(cursor, 5U);
    }
    {
        // ...normal
        String_t line(" " C1 C2 C3 " " C4 C5 C6);
        size_t cursor(7);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, " " C1 C2 C3 " " C6);
        TS_ASSERT_EQUALS(cursor, 5U);
    }
    {
        // ...into protected
        String_t line(" " C1 C2 C3 " " C6);
        size_t cursor(5);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, " " C1 C2 C6);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        // ...in protected
        String_t line(" " C1 C2 C3 " " C6);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 3, ed::Flags_t(), ed::cDeleteWordBackward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, " " C1 C2 C3 " " C6);
        TS_ASSERT_EQUALS(cursor, 1U);
    }

    // Delete word forward
    {
        // ...NonEditable
        String_t line(C1 C2 C3 C4 " " C5 C6);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(ed::NonEditable), ed::cDeleteWordForward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4 " " C5 C6);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3 C4 " " C5 C6);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 " " C5 C6);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        // ...normal, too
        String_t line(C1 C2 C3 " " C5 C6);
        size_t cursor(3);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        // ...protected
        String_t line(C1 C2 C3 C4 " " C5 C6);
        size_t cursor(1);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cDeleteWordForward, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4 " " C5 C6);
        TS_ASSERT_EQUALS(cursor, 1U);
    }

    // Transpose
    {
        // ...NonEditable
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(ed::NonEditable), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...normal
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C3 C2 C4);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
    {
        // ...start
        String_t line(C1 C2 C3 C4);
        size_t cursor(0);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C2 C1 C3 C4);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
    {
        // ...end
        String_t line(C1 C2 C3 C4);
        size_t cursor(4);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 0, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C4 C3);
        TS_ASSERT_EQUALS(cursor, 4U);
    }
    {
        // ...protection
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C4 C3);
        TS_ASSERT_EQUALS(cursor, 4U);
    }
    {
        // ...too short
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cTransposeCharacters, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 2U);
    }

    // Null
    {
        // ...too short
        String_t line(C1 C2 C3);
        size_t cursor(2);
        TS_ASSERT_EQUALS(ed::handleCommand(line, cursor, 2, ed::Flags_t(), ed::cNull, String_t::npos), true);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 2U);
    }
}

void
TestUtilEditorCommand::testHandleInsert()
{
    // Not editable
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::NonEditable), C4, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 1U);
    }

    // TypeErase, normal case
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::TypeErase), C4 C5, String_t::npos);
        TS_ASSERT_EQUALS(line, C4 C5);
        TS_ASSERT_EQUALS(cursor, 2U);
    }

    // TypeErase, with protected part
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 1, ed::Flags_t(ed::TypeErase), C4 C5, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C4 C5);
        TS_ASSERT_EQUALS(cursor, 3U);
    }

    // TypeErase, with length limit and protected part
    {
        String_t line(C1 C2 C3);
        size_t cursor(0);
        ed::handleInsert(line, cursor, 2, ed::Flags_t(ed::TypeErase), C4 C5 C6, 5);
        TS_ASSERT_EQUALS(line, C1 C2 C4 C5 C6);
        TS_ASSERT_EQUALS(cursor, 5U);
    }

    // Normal, cursor in protected part (cannot insert here)
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 2, ed::Flags_t(), C4 C5, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C2 C3);
        TS_ASSERT_EQUALS(cursor, 1U);
    }

    // Normal
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 1, ed::Flags_t(), C4 C5, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C4 C5 C2 C3);
        TS_ASSERT_EQUALS(cursor, 3U);
    }

    // Cursor after end
    {
        String_t line(C1 C2 C3);
        size_t cursor(5);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(), C4 C5, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C2 C3 "  " C4 C5);
        TS_ASSERT_EQUALS(cursor, 7U);
    }

    // Length limit
    {
        String_t line(C1 C2 C3);
        size_t cursor(2);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(), C4 C5 C6, 5);
        TS_ASSERT_EQUALS(line, C1 C2 C4 C5 C3);
        TS_ASSERT_EQUALS(cursor, 4U);
    }

    // Over limit
    {
        String_t line(C1 C2 C3 C4);
        size_t cursor(2);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(), C5 C6, 3);
        TS_ASSERT_EQUALS(line, C1 C2 C3 C4);
        TS_ASSERT_EQUALS(cursor, 2U);
    }

    // Overwrite
    {
        String_t line(C1 C2 C3);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), C4 C5 C6, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C4 C5 C6);
        TS_ASSERT_EQUALS(cursor, 4U);
    }

    // Overwrite by shorter runes
    {
        String_t line(C1 C2 C3 C4);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), "ab", String_t::npos);
        TS_ASSERT_EQUALS(line, C1 "ab" C4);
        TS_ASSERT_EQUALS(cursor, 3U);
    }

    // Overwrite by longer runes
    {
        String_t line(C1 "ab" C4);
        size_t cursor(1);
        ed::handleInsert(line, cursor, 0, ed::Flags_t(ed::Overwrite), C5 C6, String_t::npos);
        TS_ASSERT_EQUALS(line, C1 C5 C6 C4);
        TS_ASSERT_EQUALS(cursor, 3U);
    }
}

