/**
  *  \file util/editor/command.cpp
  *  \brief Editor Commands
  */

#include "util/editor/command.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/char.hpp"

namespace {
    bool isWordCharacter(afl::charset::Unichar_t ch)
    {
        // Oops, we don't have a Unicode classification function yet?
        // Treat everything outside ASCII as word character.
        return ch >= 0x80
            || afl::string::charIsAlphanumeric(static_cast<char>(ch));
    }

    void moveWordLeft(const String_t& line, size_t& cursor, size_t limit)
    {
        afl::charset::Utf8 u8(0);
        while (cursor > limit && !isWordCharacter(u8.charAt(line, cursor-1))) {
            --cursor;
        }
        while (cursor > limit && isWordCharacter(u8.charAt(line, cursor-1))) {
            --cursor;
        }
    }

    void moveWordRight(const String_t& line, size_t& cursor, size_t limit)
    {
        afl::charset::Utf8 u8(0);
        while (cursor < limit && !isWordCharacter(u8.charAt(line, cursor))) {
            ++cursor;
        }
        while (cursor < limit && isWordCharacter(u8.charAt(line, cursor))) {
            ++cursor;
        }
    }

    void deleteLine(String_t& line, size_t& cursor, size_t protectUntil)
    {
        afl::charset::Utf8 u8(0);
        line.erase(u8.charToBytePos(line, protectUntil));
        cursor = u8.length(line);
    }

    void deleteCharacter(String_t& line, size_t& cursor)
    {
        afl::charset::Utf8 u8(0);
        size_t pos1 = u8.charToBytePos(line, cursor);
        size_t pos2 = u8.charToBytePos(line, cursor+1);
        line.erase(pos1, pos2-pos1);
    }

    void limitText(String_t& text, size_t cursor, size_t lengthLimit)
    {
        if (cursor >= lengthLimit) {
            // Already over limit
            text.clear();
        } else {
            // Truncate
            afl::charset::Utf8 u8(0);
            text.erase(u8.charToBytePos(text, lengthLimit - cursor));
        }
    }
}


const char*
util::editor::toString(Command c)
{
    switch (c) {
     case cMoveLineUp:                 return "MoveLineUp";
     case cMoveLineDown:               return "MoveLineDown";
     case cMoveCharacterLeft:          return "MoveCharacterLeft";
     case cMoveCharacterRight:         return "MoveCharacterRight";
     case cMoveWordLeft:               return "MoveWordLeft";
     case cMoveWordRight:              return "MoveWordRight";
     case cMoveBeginningOfLine:        return "MoveBeginningOfLine";
     case cMoveEndOfLine:              return "MoveEndOfLine";
     case cMoveBeginningOfDocument:    return "MoveBeginningOfDocument";
     case cMoveEndOfDocument:          return "MoveEndOfDocument";
     case cDeleteCharacter:            return "DeleteCharacter";
     case cDeleteCharacterBackward:    return "DeleteCharacterBackward";
     case cDeleteLine:                 return "DeleteLine";
     case cDeleteEndOfLine:            return "DeleteEndOfLine";
     case cDeleteWordBackward:         return "DeleteWordBackward";
     case cDeleteWordForward:          return "DeleteWordForward";
     case cTransposeCharacters:        return "TransposeCharacters";
     case cToggleInsert:               return "ToggleInsert";
     case cToggleWrap:                 return "ToggleWrap";
     case cInsertTab:                  return "InsertTab";
     case cInsertNewline:              return "InsertNewline";
     case cInsertNewlineAbove:         return "InsertNewlineAbove";
     case cNull:                       return "Null";
    }
    return 0;
}

bool
util::editor::lookupKey(Key_t key, Command& c)
{
    switch (key) {
     case Key_Up:                             c = cMoveLineUp;              return true;
     case Key_Down:                           c = cMoveLineDown;            return true;
     case Key_Left:                           c = cMoveCharacterLeft;       return true;
     case Key_Right:                          c = cMoveCharacterRight;      return true;
     case Key_Left + KeyMod_Ctrl:             c = cMoveWordLeft;            return true;
     case Key_Right + KeyMod_Ctrl:            c = cMoveWordRight;           return true;
     case Key_Home:                           c = cMoveBeginningOfLine;     return true;
     case 'a' + KeyMod_Ctrl:                  c = cMoveBeginningOfLine;     return true;
     case Key_End:                            c = cMoveEndOfLine;           return true;
     case 'e' + KeyMod_Ctrl:                  c = cMoveEndOfLine;           return true;
     case Key_Home + KeyMod_Ctrl:             c = cMoveBeginningOfDocument; return true;
     case Key_End + KeyMod_Ctrl:              c = cMoveEndOfDocument;       return true;
     case Key_Delete:                         c = cDeleteCharacter;         return true;
     case 'd' + KeyMod_Ctrl:                  c = cDeleteCharacter;         return true;
     case Key_Backspace:                      c = cDeleteCharacterBackward; return true;
     case 'y' + KeyMod_Ctrl:                  c = cDeleteLine;              return true;
     case 'k' + KeyMod_Ctrl:                  c = cDeleteEndOfLine;         return true;
     case Key_Backspace + KeyMod_Ctrl:        c = cDeleteWordBackward;      return true;
     case 'd' + KeyMod_Alt:                   c = cDeleteWordForward;       return true;
     case 't' + KeyMod_Ctrl:                  c = cTransposeCharacters;     return true;
     case Key_Insert:                         c = cToggleInsert;            return true;
     case 'w' + KeyMod_Alt:                   c = cToggleWrap;              return true;
     case Key_Tab:                            c = cInsertTab;               return true;
     case Key_Return:                         c = cInsertNewline;           return true;
     case 'n' + KeyMod_Ctrl:                  c = cInsertNewlineAbove;      return true;
     default:                                                               return false;
    }
}

bool
util::editor::handleCommand(String_t& line, size_t& cursor, size_t protectUntil, Flags_t flags, Command c, size_t lengthLimit)
{
    afl::charset::Utf8 u8(0);
    switch (c) {
     case cMoveLineUp:
     case cMoveLineDown:
        // Not a single-line command
        return false;

     case cMoveCharacterLeft:
        if (cursor > 0) {
            --cursor;
        }
        return true;

     case cMoveCharacterRight:
        if (cursor < u8.length(line) || (flags.contains(AllowCursorAfterEnd) && cursor < lengthLimit)) {
            ++cursor;
        }
        return true;

     case cMoveWordLeft:
        cursor = std::min(cursor, u8.length(line));
        moveWordLeft(line, cursor, 0);
        return true;

     case cMoveWordRight:
        moveWordRight(line, cursor, u8.length(line));
        return true;

     case cMoveBeginningOfLine:
        if (cursor > protectUntil) {
            cursor = protectUntil;
        } else if (cursor > 0) {
            cursor = 0;
        }
        return true;

     case cMoveEndOfLine:
        cursor = u8.length(line);
        return true;

     case cMoveBeginningOfDocument:
     case cMoveEndOfDocument:
        return false;

     case cDeleteCharacter:
        if (flags.contains(NonEditable)) {
            // Nothing
        } else if (flags.contains(TypeErase)) {
            // Delete entire (unprotected) range
            deleteLine(line, cursor, protectUntil);
        } else if (cursor >= protectUntil) {
            // Delete single character
            deleteCharacter(line, cursor);
        } else {
            // Cursor in protected area
        }
        return true;

     case cDeleteCharacterBackward:
        if (flags.contains(NonEditable)) {
            // Nothing
            if (cursor > 0) {
                --cursor;
            }
        } else if (flags.contains(TypeErase)) {
            // Delete entire (unprotected) range
            deleteLine(line, cursor, protectUntil);
        } else if (cursor > protectUntil) {
            // Delete single character
            // FIXME: if overwrite enabled, replace by ' '?
            --cursor;
            deleteCharacter(line, cursor);
        } else {
            // Cursor in protected area, just go back
            if (cursor > 0) {
                --cursor;
            }
        }
        return true;

     case cDeleteLine:
        if (flags.contains(NonEditable)) {
            // Nothing
        } else {
            // Delete entire (unprotected) range
            deleteLine(line, cursor, protectUntil);
        }
        return true;

     case cDeleteEndOfLine:
        if (flags.contains(NonEditable)) {
            // Nothing
        } else {
            // Treat left of cursor as protected and delete
            deleteLine(line, cursor, std::max(cursor, protectUntil));
        }
        return true;

     case cDeleteWordBackward:
        if (!flags.contains(NonEditable) && cursor > protectUntil) {
            // Delete
            size_t pos2 = u8.charToBytePos(line, cursor);
            moveWordLeft(line, cursor, protectUntil);
            cursor = std::max(cursor, protectUntil);
            size_t pos1 = u8.charToBytePos(line, cursor);
            line.erase(pos1, pos2-pos1);
        } else {
            // In protected area; just go backward
            moveWordLeft(line, cursor, 0);
        }
        return true;

     case cDeleteWordForward:
        if (!flags.contains(NonEditable) && cursor >= protectUntil) {
            // Delete
            size_t pos1 = u8.charToBytePos(line, cursor);
            size_t tmp = cursor;
            moveWordRight(line, tmp, u8.length(line));
            size_t pos2 = u8.charToBytePos(line, tmp);
            line.erase(pos1, pos2-pos1);
        } else {
            // In protected area
        }
        return true;

     case cTransposeCharacters:
        if (flags.contains(NonEditable)) {
            // Protected
        } else {
            size_t limit = u8.length(line);
            if (protectUntil < limit && limit - protectUntil >= 2 && cursor >= protectUntil) {
                // We have at least 2 unprotected characters, and cursor is there.
                // Make sure cursor is not at first or last character, exactly.
                if (cursor == protectUntil) {
                    ++cursor;
                }
                if (cursor >= limit-1) {
                    cursor = limit-1;
                }

                // Shuffle around
                size_t pos1 = u8.charToBytePos(line, cursor-1);
                size_t pos2 = u8.charToBytePos(line, cursor);
                size_t pos3 = u8.charToBytePos(line, cursor+1);
                String_t first(line, pos1, pos2-pos1);
                line.erase(pos1, pos2-pos1);
                line.insert(pos1 + (pos3-pos2), first);

                ++cursor;
            }
        }
        return true;

     case cToggleInsert:
     case cToggleWrap:
     case cInsertTab:
     case cInsertNewline:
     case cInsertNewlineAbove:
        return false;

     case cNull:
        return true;
    }
    return false;
}

void
util::editor::handleInsert(String_t& line,
                           size_t& cursor,
                           size_t protectUntil,
                           Flags_t flags,
                           String_t text,
                           size_t lengthLimit)
{
    afl::charset::Utf8 u8(0);
    if (flags.contains(NonEditable)) {
        // Ignore
    } else if (flags.contains(TypeErase)) {
        // Type-erase: replace (editable) part by new text
        deleteLine(line, cursor, protectUntil);
        limitText(text, 0, lengthLimit);
        line += text;
        cursor += u8.length(text);
    } else if (cursor < protectUntil) {
        // Cursor in protected area
    } else {
        // Valid
        // Expand to cursor position
        size_t lineLen = u8.length(line);
        if (lineLen < cursor) {
            line.append(cursor-lineLen, ' ');
            lineLen = cursor;
        }

        // Limit text to insert if required
        limitText(text, lineLen, lengthLimit);

        // Find insertion point
        size_t textLen = u8.length(text);
        size_t pos1 = u8.charToBytePos(line, cursor);
        if (flags.contains(Overwrite)) {
            size_t pos2 = u8.charToBytePos(line, cursor+textLen);
            line.erase(pos1, pos2-pos1);
        }

        // Perform insertion
        line.insert(pos1, text);
        cursor += textLen;
    }
}
