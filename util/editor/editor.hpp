/**
  *  \file util/editor/editor.hpp
  *  \brief Class util::editor::Editor
  */
#ifndef C2NG_UTIL_EDITOR_EDITOR_HPP
#define C2NG_UTIL_EDITOR_EDITOR_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "util/editor/command.hpp"

namespace util { namespace editor {

    /** Multi-line text editor.
        Implements a text editor consisting of an array of lines.
        Each line has:
        - a possible protected prefix that cannot be edited
        - unicode text content
        - continuation marker for word-wrap

        Editor status:
        - cursor position (assumes that all characters have the same size)
        - maximum line length; user input cannot grow lines any longer
        - maximum number of lines; excess lines can be discarded

        External editor status is kept in a Flags_t object passed in per-call:
        - AllowCursorAfterEnd
        - Overwrite
        - WordWrap */
    class Editor {
     public:
        /** Constructor.
            Makes empty editor. */
        Editor();

        /** Destructor. */
        ~Editor();

        /** Set line content.
            This is the basic operation to define the editor's initial content.
            Parameters are not validated against limits.
            The line will have no protected area, and no continuation.

            \param line Line number (0-based)
            \param text Text */
        void setLine(size_t line, String_t text);

        /** Set line content, full version.
            Parameters are not validated against limits.

            \param line            Line number (0-based)
            \param text            Text
            \param protectUntil    Number of Unicode characters in this line that are not editable.
                                   Can be larger than Utf8::length(text) (e.g., npos) to make the line entirely immutable.
            \param hasContinuation true if this line is part of a paragraph that continues with the next line */
        void setLine(size_t line, String_t text, size_t protectUntil, bool hasContinuation);

        /** Set length limit.
            Lines will not be allowed to grow over this limit.
            If WordWrap is enabled, lines will be wrapped to that limit.
            \param n New limit */
        void setLengthLimit(size_t n);

        /** Set limit to number of lines.
            This is an (approximate) limit to the maximum number of lines.
            Lines after this limit will be discarded.
            \param n New limit */
        void setLineLimit(size_t n);

        /** Set line number limitations for user movement.
            Users cannot go above the minimum line or below the maximum line.
            \param min Minimum line
            \param max Maximum line */
        void setUserLineLimit(size_t min, size_t max);

        /** Set cursor position.
            Parameters are not validated against limits.

            \param line    Line (0-based)
            \param column  Column (0-based) */
        void setCursor(size_t line, size_t column);

        /** Get number of lines.
            getLineText() will return "" for all lines >= this value.
            \return number of lines */
        size_t getNumLines() const;

        /** Get current line number.
            \return current line number (0-based)
            \see setCursor() */
        size_t getCurrentLine() const;

        /** Get current column number.
            \return current column number (0-based)
            \see setCursor() */
        size_t getCurrentColumn() const;

        /** Get line length limit.
            \return limit
            \see setLengthLimit */
        size_t getLengthLimit() const;

        /** Get number of lines limit.
            \return limit
            \see setLineLimit */
        size_t getLineLimit() const;

        /** Get text contained in a line.
            \param line line number (0-based)
            \return text */
        String_t getLineText(size_t line) const;

        /** Get range of text.
            \param firstLine   First line to copy
            \param firstColumn First column in first line to copy
            \param lastLine    Last line to copy
            \param lastColumn  First column in last line to NOT copy
            \return section of text, with lines separated by newlines */
        String_t getRange(size_t firstLine, size_t firstColumn, size_t lastLine, size_t lastColumn) const;

        /** Insert new empty lines.
            If the cursor is sitting below the insertion, it is moved down.
            \param beforeLine  Insert new lines before this line
            \param numLines    Number of lines to insert */
        void insertLine(size_t beforeLine, size_t numLines);

        /** Delete lines.
            If the cursor is sitting on or below the deletion, it is moved up.
            \param line        Remove this line
            \param numLines    Number of lines to remove */
        void deleteLine(size_t line, size_t numLines);

        /** Handle command.
            \param flags Flags
            \param c     Command
            \return true if command was handled
            \see util::editor::handleCommand() */
        bool handleCommand(Flags_t flags, Command c);

        /** Handle insertion of text.
            \param flags Flags
            \param text  Text, can include "\t" */
        void handleInsert(Flags_t flags, String_t text);

        /** Signal: change to content or cursor.

            Modifying a line will mark that single line changed (min=max).
            Inserting a line will report that line and all others up to the end as changed.
            The maximum line can be larger than the number of lines this editor has.

            \param min Minimum line (inclusive)
            \param max Maximum line (inclusive) */
        afl::base::Signal<void(size_t, size_t)> sig_change;

     private:
        struct Line {
            size_t protectUntil;
            String_t text;
            bool hasContinuation;

            Line(size_t protectUntil, String_t text, bool hasContinuation)
                : protectUntil(protectUntil), text(text), hasContinuation(hasContinuation)
                { }
        };

        struct Notifier;

        afl::container::PtrVector<Line> m_lines;
        size_t m_currentLine;
        size_t m_currentColumn;
        size_t m_lengthLimit;         ///< Maximum number of columns in each line.
        size_t m_lineLimit;           ///< Maximum line number (=maximum number of lines, minus 1).
        size_t m_minLine;             ///< Minimum user-accessible line.
        size_t m_maxLine;             ///< Maximum user-accessible line.

        Notifier start();
        bool finish(Notifier& n);
        void modifyLine(Notifier& n, size_t line);
        void modifyEnd(Notifier& n, size_t line);

        bool handleSingleLineCommand(Flags_t flags, Command c);
        bool checkDeleteForward(Notifier& n);
        bool checkDeleteBackward(Notifier& n);
        void deleteLine(Notifier& n, size_t line);
        Line& insertLine(Notifier& n, size_t beforeLine, String_t text, bool hasContinuation);

        void handleInsertTab(Notifier& n, Flags_t flags);
        void breakCurrentLine(Notifier& n);
        bool insertNewline(Notifier& n);
        void insertText(Flags_t flags, Notifier& n, const String_t& text);

        void wrapLine(Notifier& n, size_t line);
        bool hasProtectedPrefix(size_t n) const;
        bool isProtectedLine(size_t n) const;
        size_t skipProtectedLines(size_t startAt) const;
        void trimLines();
        void limitColumn(Flags_t flags);

        Line& getLine(size_t line);
        size_t getLineLength(size_t line) const;
    };

} }

#endif
