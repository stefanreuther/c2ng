/**
  *  \file util/editor/command.hpp
  *  \brief Editor Commands
  */
#ifndef C2NG_UTIL_EDITOR_COMMAND_HPP
#define C2NG_UTIL_EDITOR_COMMAND_HPP

#include <cstddef>
#include "afl/bits/smallset.hpp"
#include "afl/string/string.hpp"
#include "util/key.hpp"

namespace util { namespace editor {

    /** Editor command.
        Represents any one of our editor commands for single-line and multi-line editors. */
    enum Command {
        cMoveLineUp,                     // Up
        cMoveLineDown,                   // Down
        cMoveCharacterLeft,              // Left
        cMoveCharacterRight,             // Right
        cMoveWordLeft,                   // C-Left
        cMoveWordRight,                  // C-Right
        cMoveBeginningOfLine,            // Home, C-a
        cMoveEndOfLine,                  // End, C-e
        cMoveBeginningOfDocument,        // C-Home [PCC1
        cMoveEndOfDocument,              // C-End [PCC1]
        cDeleteCharacter,                // Del, C-d
        cDeleteCharacterBackward,        // Backspace
        cDeleteLine,                     // C-y
        cDeleteEndOfLine,                // C-k
        cDeleteWordBackward,             // C-Backspace
        cDeleteWordForward,              // C-d [PCC1], M-d
        cTransposeCharacters,            // C-t
        cToggleInsert,                   // Ins [PCC1]
        cToggleWrap,                     // M-w [PCC1]
        cInsertTab,                      // Tab [PCC1]
        cInsertNewline,                  // Enter
        cInsertNewlineAbove,             // C-n
        cNull                            // C-u (does nothing; used for clearing TypeErase). Let's keep this last.
    };
    static const size_t NUM_COMMANDS = static_cast<size_t>(cNull) + 1;

    /** Editor status flag. */
    enum Flag {
        AllowCursorAfterEnd,             ///< Allow placing cursor after end of a line.
        TypeErase,                       ///< Typing/modifying will erase the line.
        NonEditable,                     ///< Line is not editable.
        Overwrite,                       ///< Overwrite mode is active.
        WordWrap                         ///< Word wrap mode is active.
    };
    typedef afl::bits::SmallSet<Flag> Flags_t;

    /** Format editor command to string.
        \param c Command
        \return Name of command, null if invalid */
    const char* toString(Command c);

    /** Look up a key, default (hardwired) keymap.
        \param [in]  key Key
        \param [out] c   Command
        \return true if key was mapped to a command */
    bool lookupKey(Key_t key, Command& c);

    /** Process a command.

        This implements a common foundation for single-line and multi-line editors.
        Represents editing in a UTF-8 string consisting of
        - an optional protected/non-editable area (e.g. field label), given by its length in characters (0=none)
        - an editable area
        limited to a total length limit (String_t::npos=none).

        Basic sequence of events for a command:
        - convert keystroke to command. This can use the lookupKey() function,
          but could also use a configurable keymap implemented outside this module.
        - for a multiline editor, check whether you can handle the command.
          This applies to vertical movement commands, but you may want to handle others as well
          (e.g., cDeleteEndOfLine should join lines if cursor is at EOL).
        - likewise, state commands (cToggleWrap etc.) must be handled outside.
        - if you decide to handle it as a single-line command, call handleCommand(),
          passing it appropriate values for all parameters.

        \param [in/out]  line         Line of text being edited
        \param [in/out]  cursor       Cursor position (character index, NOT byte index)
        \param [in]      protectUntil Do not allow editing characters before this index
        \param [in]      flags        Additional status flags
        \param [in]      c            Command
        \param [in]      lengthLimit  Line length limit
        \retval true   Command was handled
        \retval false  Command could not be interpreted as single-line editing command */
    bool handleCommand(String_t& line,
                       size_t& cursor,
                       size_t protectUntil,
                       Flags_t flags,
                       Command c,
                       size_t lengthLimit);

    /** Process an insertion.

        Inserts the given text as if each unicode character were being inserted one after another
        (that is, if the length limit is exceeded, it may be inserted partially).

        For general usage notes, see handleCommand().

        Character insertion is not modelled as a Command to allow users to classify characters themselves as needed
        (e.g. allow only ASCII characters to be typed).

        \param [in/out]  line         Line of text being edited
        \param [in/out]  cursor       Cursor position (character index, NOT byte index)
        \param [in]      protectUntil Do not allow editing characters before this index
        \param [in]      flags        Additional status flags
        \param [in]      text         Text to insert
        \param [in]      lengthLimit  Line length limit */
    void handleInsert(String_t& line,
                      size_t& cursor,
                      size_t protectUntil,
                      Flags_t flags,
                      String_t text,
                      size_t lengthLimit);

} }

#endif
