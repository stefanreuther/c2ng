/**
  *  \file util/editor/editor.cpp
  *  \brief Class util::editor::Editor
  */

#include <algorithm>
#include "util/editor/editor.hpp"
#include "afl/charset/utf8.hpp"

using afl::charset::Utf8;
using afl::charset::Unichar_t;

namespace {
    const size_t nil = size_t(-1);

    const size_t TAB_SIZE = 8;

    bool isSpace(Unichar_t ch)
    {
        return ch == ' ' || ch == 0;
    }

    bool isSeparator(Unichar_t ch)
    {
        return ch == '-' || ch == '/';
    }
}

struct util::editor::Editor::Notifier {
    size_t first, last;
};


util::editor::Editor::Editor()
    : m_lines(),
      m_currentLine(0),
      m_currentColumn(0),
      m_lengthLimit(nil),
      m_lineLimit(nil),
      m_minLine(0),
      m_maxLine(nil)
{ }

util::editor::Editor::~Editor()
{ }

void
util::editor::Editor::setLine(size_t line, String_t text)
{
    setLine(line, text, 0, false);
}

void
util::editor::Editor::setLine(size_t line, String_t text, size_t protectUntil, bool hasContinuation)
{
    size_t first = m_lines.size();
    while (m_lines.size() <= line) {
        m_lines.pushBackNew(new Line(0, String_t(), false));
    }
    m_lines[line]->text = text;
    m_lines[line]->protectUntil = protectUntil;
    m_lines[line]->hasContinuation = hasContinuation;
    if (first < line) {
        sig_change.raise(first, line);
    } else {
        sig_change.raise(line, line);
    }
}

void
util::editor::Editor::setLengthLimit(size_t n)
{
    m_lengthLimit = n;
}

void
util::editor::Editor::setLineLimit(size_t n)
{
    m_lineLimit = n;
}

void
util::editor::Editor::setUserLineLimit(size_t min, size_t max)
{
    // ex WEditor::setUserLineRestrictions
    m_minLine = min;
    m_maxLine = max;
}

void
util::editor::Editor::setCursor(size_t line, size_t column)
{
    if (line != m_currentLine || column != m_currentColumn) {
        Notifier n = start();
        m_currentLine = line;
        m_currentColumn = column;
        finish(n);
    }
}

size_t
util::editor::Editor::getNumLines() const
{
    return m_lines.size();
}

size_t
util::editor::Editor::getCurrentLine() const
{
    return m_currentLine;
}

size_t
util::editor::Editor::getCurrentColumn() const
{
    return m_currentColumn;
}

size_t
util::editor::Editor::getLengthLimit() const
{
    return m_lengthLimit;
}

size_t
util::editor::Editor::getLineLimit() const
{
    return m_lineLimit;
}

String_t
util::editor::Editor::getLineText(size_t line) const
{
    if (line < m_lines.size()) {
        return m_lines[line]->text;
    } else {
        return String_t();
    }
}

String_t
util::editor::Editor::getRange(size_t firstLine, size_t firstColumn, size_t lastLine, size_t lastColumn) const
{
    if (firstLine > lastLine) {
        return String_t();
    } else {
        Utf8 u8(0);
        String_t result = getLineText(firstLine);
        if (firstLine == lastLine) {
            result.erase(u8.charToBytePos(result, lastColumn));
            result.erase(0, u8.charToBytePos(result, firstColumn));
        } else {
            result.erase(0, u8.charToBytePos(result, firstColumn));

            size_t i = firstLine+1;
            while (i < lastLine) {
                result += "\n";
                result += getLineText(i);
                ++i;
            }

            result += "\n";
            if (lastColumn != 0) {
                String_t last = getLineText(lastLine);
                last.erase(u8.charToBytePos(last, lastColumn));
                result += last;
            }
        }
        return result;
    }
}

void
util::editor::Editor::insertLine(size_t beforeLine, size_t numLines)
{
    // ex editor.inc:InsLine
    // Unoptimized and simple, assuming that dimensions are small.
    if (numLines > 0) {
        Notifier n = start();
        while (m_lines.size() < beforeLine) {
            insertLine(n, m_lines.size(), String_t(), false);
        }
        for (size_t i = 0; i < numLines; ++i) {
            insertLine(n, beforeLine, String_t(), false);
        }
        if (m_currentLine >= beforeLine) {
            m_currentLine += numLines;
        }
        finish(n);
    }
}

void
util::editor::Editor::deleteLine(size_t line, size_t numLines)
{
    // ex editor.inc:DeleteLine
    // Unoptimized and simple, assuming that dimensions are small.
    if (numLines > 0) {
        Notifier n = start();
        while (m_lines.size() > line && numLines > 0) {
            deleteLine(n, line);
            if (m_currentLine > line) {
                --m_currentLine;
            }
            --numLines;
        }
        finish(n);
    }
}

bool
util::editor::Editor::handleCommand(Flags_t flags, Command c)
{
    /* Multi-line commands handled here with 'return true', single-line commands fall through.
       Commands must always be handled entirely or not at all (no conditional fall-through). */
    Notifier n;
    switch (c) {
     case cMoveLineUp:
        // Move up: simple
        if (m_currentLine > m_minLine) {
            --m_currentLine;
            limitColumn(flags);
            sig_change.raise(m_currentLine, m_currentLine+1);
        }
        return true;

     case cMoveLineDown:
        // Move down: simple
        // FIXME: decide upon rules when lines are created.
        // For now, a line is created when the cursor moves to it (the getLine() call).
        // Alternatives:
        // - only allow cInsertNewline/cInsertNewlineAbove to insert newlines and limit cursor to m_lines.size()
        // - ignore the problem (auto-create lines, and leave them there...)
        // - auto-create and auto-delete lines */
        if (m_currentLine < m_lineLimit && m_currentLine < m_maxLine) {
            ++m_currentLine;
            getLine(m_currentLine);
            limitColumn(flags);
            sig_change.raise(m_currentLine-1, m_currentLine);
        }
        return true;

     case cMoveCharacterLeft:
     case cMoveCharacterRight:
        // in single-line handler
        break;

     case cMoveWordLeft:
        // Word left: when at start of line, move to previous
        n = start();
        if (m_currentColumn == 0 && m_currentLine > m_minLine) {
            --m_currentLine;
            m_currentColumn = getLineLength(m_currentLine);
        }
        handleSingleLineCommand(flags, c);
        return finish(n);

     case cMoveWordRight:
        // Word right: when at end of line, move to next
        n = start();
        if (m_currentLine < m_lineLimit && m_currentLine < m_maxLine && m_currentColumn >= getLineLength(m_currentLine)) {
            ++m_currentLine;
            m_currentColumn = 0;
        }
        handleSingleLineCommand(flags, c);
        return finish(n);

     case cMoveBeginningOfLine:
     case cMoveEndOfLine:
        // in single-line handler
        break;

     case cMoveBeginningOfDocument:
        // Beginning: go to first editable character
        n = start();
        m_currentLine = skipProtectedLines(m_minLine);
        m_currentColumn = (m_currentLine < m_lines.size() ? m_lines[m_currentLine]->protectUntil : 0);
        return finish(n);

     case cMoveEndOfDocument:
        // End: go to last existing line
        n = start();
        if (m_lines.empty()) {
            m_currentLine = 0;
            m_currentColumn = 0;
        } else {
            m_currentLine = std::min(m_maxLine, m_lines.size() - 1);
            m_currentColumn = getLineLength(m_currentLine);
        }
        return finish(n);

     case cDeleteCharacter:
        // Delete character forward: if at end of line, join lines; otherwise, delete normally.
        n = start();
        if (checkDeleteForward(n)) {
            wrapLine(n, m_currentLine);
        } else {
            handleSingleLineCommand(flags, c);
        }
        return finish(n);

     case cDeleteCharacterBackward:
        // Delete character backward: if at beginning of line, join lines; otherwise, delete normally
        n = start();
        if (checkDeleteBackward(n)) {
            wrapLine(n, m_currentLine);
        } else {
            handleSingleLineCommand(flags, c);
        }
        return finish(n);

     case cDeleteLine:
        // Delete line: if line has no protected part, delete it entirely. Otherwise, just the modifyable part (normal single-line operation)
        // FIXME: do not allow removing the free line in a "protected/free/protected" sequence because users can not add that again
        n = start();
        if (m_currentLine < m_lines.size() && !hasProtectedPrefix(m_currentLine)) {
            deleteLine(n, m_currentLine);
            handleSingleLineCommand(flags, cMoveBeginningOfLine);
        } else {
            handleSingleLineCommand(flags, c);
        }
        return finish(n);

     case cDeleteEndOfLine:
        // Delete end of line: if at end of line, join lines; otherwise, delete normally.
        n = start();
        if (checkDeleteForward(n)) {
            wrapLine(n, m_currentLine);
        } else {
            handleSingleLineCommand(flags, c);
        }
        return finish(n);

     case cDeleteWordBackward:
        // Delete word backward: if at beginning of line, join lines, then delete normally.
        n = start();
        checkDeleteBackward(n);
        handleSingleLineCommand(flags, c);
        wrapLine(n, m_currentLine);
        return finish(n);

     case cDeleteWordForward:
        // Delete word forward: if at end of line, join lines, then delete normally.
        n = start();
        checkDeleteForward(n);
        handleSingleLineCommand(flags, c);
        wrapLine(n, m_currentLine);
        return finish(n);

     case cTransposeCharacters:
        // in single-line handler
        break;

     case cToggleInsert:
     case cToggleWrap:
        // in external driver
        break;

     case cInsertTab:
        // Insert tab
        n = start();
        if (!isProtectedLine(m_currentLine)) {
            handleInsertTab(n, flags);
        }
        return finish(n);

     case cInsertNewline:
        // Insert new line: allowed if not both this and next line are protected
        n = start();
        insertNewline(n);
        return finish(n);

     case cInsertNewlineAbove:
        // Insert new line above: allowed if not both this and previous line are protected
        n = start();
        if (!hasProtectedPrefix(m_currentLine)
            || (m_currentLine > 0 && !hasProtectedPrefix(m_currentLine-1)))
        {
            insertLine(n, m_currentLine, String_t(), false);
            trimLines();
        }
        return finish(n);

     case cNull:
        // in single-line handler
        break;
    }

    bool ok = handleSingleLineCommand(flags, c);
    if (ok) {
        sig_change.raise(m_currentLine, m_currentLine);
    }
    return ok;
}

void
util::editor::Editor::handleInsert(Flags_t flags, String_t text)
{
    Notifier n = start();
    size_t pos = 0, p;
    while ((p = text.find('\n', pos)) != String_t::npos) {
        insertText(flags, n, text.substr(pos, p - pos));
        if (!insertNewline(n)) {
            insertText(flags, n, " ");
        }
        pos = p+1;
    }
    insertText(flags, n, text.substr(pos));
    trimLines();
    finish(n);
}

util::editor::Editor::Notifier
util::editor::Editor::start()
{
    Notifier n = {m_currentLine, m_currentLine};
    return n;
}

bool
util::editor::Editor::finish(Notifier& n)
{
    modifyLine(n, m_currentLine);
    sig_change.raise(n.first, n.last);
    return true;
}

void
util::editor::Editor::modifyLine(Notifier& n, size_t line)
{
    n.first = std::min(n.first, line);
    n.last  = std::max(n.last,  line);
}

void
util::editor::Editor::modifyEnd(Notifier& n, size_t line)
{
    n.first = std::min(n.first, line);
    n.last  = nil;
}

bool
util::editor::Editor::handleSingleLineCommand(Flags_t flags, Command c)
{
    Line& line = getLine(m_currentLine);
    bool ok = util::editor::handleCommand(line.text, m_currentColumn, line.protectUntil, flags, c, m_lengthLimit);

    // Clearing a line resets its hasContinuation status to avoid surprises
    if (line.text.empty()) {
        line.hasContinuation = false;
    }
    return ok;
}

bool
util::editor::Editor::checkDeleteForward(Notifier& n)
{
    size_t cur = m_currentLine;
    size_t limit = m_lines.size();
    /* Current line must exist and not be all protected.
       Cursor must be at end of it.
       Next line must exist and not start with a protected area. */
    if (cur < limit
        && !isProtectedLine(cur)
        && m_currentColumn >= getLineLength(cur)
        && cur+1 < limit
        && !hasProtectedPrefix(cur+1))
    {
        // Extend to cursor
        Line& curLine = *m_lines[cur];
        size_t spacesNeeded = m_currentColumn - Utf8(0).length(curLine.text);
        curLine.text.append(spacesNeeded, ' ');

        // Append next line
        curLine.text += m_lines[cur+1]->text;
        curLine.hasContinuation = m_lines[cur+1]->hasContinuation;

        // Delete next line
        deleteLine(n, cur+1);
        return true;
    } else {
        return false;
    }
}

bool
util::editor::Editor::checkDeleteBackward(Notifier& n)
{
    size_t cur = m_currentLine;
    size_t limit = m_lines.size();
    /* Current line must exist and not start with protected text.
       Cursor must be at beginning.
       Previous line must not be all protected. */
    if (m_currentColumn == 0
        && cur < limit
        && !hasProtectedPrefix(cur)
        && cur > m_minLine
        && !isProtectedLine(cur-1))
    {
        // Concatenate
        Line& curLine = *m_lines[cur];
        Line& prevLine = *m_lines[cur-1];
        size_t prevLen = Utf8(0).length(prevLine.text);
        prevLine.text += curLine.text;

        // Move cursor
        m_currentLine = cur-1;
        m_currentColumn = prevLen;

        // Delete old line
        deleteLine(n, cur);
        return true;
    } else {
        return false;
    }
}

void
util::editor::Editor::deleteLine(Notifier& n, size_t line)
{
    m_lines.erase(m_lines.begin() + line);
    modifyEnd(n, line);
}

util::editor::Editor::Line&
util::editor::Editor::insertLine(Notifier& n, size_t beforeLine, String_t text, bool hasContinuation)
{
    Line& result = *m_lines.insertNew(m_lines.begin() + beforeLine, new Line(0, text, hasContinuation));
    modifyEnd(n, beforeLine);
    return result;
}

void
util::editor::Editor::handleInsertTab(Notifier& n, Flags_t flags)
{
    Utf8 u8(0);
    const Line& me = getLine(m_currentLine);
    if (m_currentColumn < me.protectUntil) {
        // When before label, go to beginning of field
        m_currentColumn = me.protectUntil;
    } else {
        // Normal operation
        size_t targetPos = 0;
        if (m_currentLine > 0) {
            const Line& prev = getLine(m_currentLine-1);
            size_t pos = m_currentColumn + 1;
            size_t limit = u8.length(prev.text);
            while (pos < limit && !isSpace(u8.charAt(prev.text, pos))) {
                ++pos;
            }
            while (pos < limit) {
                if (!isSpace(u8.charAt(prev.text, pos))) {
                    targetPos = pos;
                    break;
                }
                ++pos;
            }
        }
        if (targetPos == 0) {
            targetPos = (m_currentColumn+(TAB_SIZE-1)) / TAB_SIZE * TAB_SIZE;
        }
        if (targetPos > m_lengthLimit) {
            targetPos = m_lengthLimit;
        }
        if (targetPos > m_currentColumn) {
            insertText(flags, n, String_t(targetPos - m_currentColumn, ' '));
        }
    }
}

void
util::editor::Editor::breakCurrentLine(Notifier& n)
{
    Line& me = getLine(m_currentLine);
    String_t::size_type cutPos = Utf8(0).charToBytePos(me.text, m_currentColumn);
    insertLine(n, m_currentLine+1, me.text.substr(cutPos), me.hasContinuation);

    // Truncate current
    me.text.erase(cutPos);
    me.hasContinuation = false;

    // Move to next line
    if (m_currentLine < m_maxLine) {
        ++m_currentLine;
        m_currentColumn = 0;
    }
}

bool
util::editor::Editor::insertNewline(Notifier& n)
{
    if (!hasProtectedPrefix(m_currentLine) || !hasProtectedPrefix(m_currentLine+1)) {
        breakCurrentLine(n);
        trimLines();
        return true;
    } else {
        return false;
    }
}

void
util::editor::Editor::insertText(Flags_t flags, Notifier& n, const String_t& text)
{
    if (text.empty()) {
        // Ignore
    } else {
        Line& line = getLine(m_currentLine);
        if (flags.contains(WordWrap)) {
            // Wrap enabled: insert everything at once, then break
            modifyLine(n, m_currentLine);
            util::editor::handleInsert(line.text, m_currentColumn, line.protectUntil, flags, text, nil);
            wrapLine(n, m_currentLine);
        } else {
            // Wrap disabled: write until line full
            modifyLine(n, m_currentLine);
            util::editor::handleInsert(line.text, m_currentColumn, line.protectUntil, flags, text, m_lengthLimit);
        }
    }
}

void
util::editor::Editor::wrapLine(Notifier& n, size_t line)
{
    // ex editor.inc:JoinLines (sort-of)
    Utf8 u8(0);
    while (line < m_lines.size()) {
        Line& me = *m_lines[line];

        // Save UTF counting if limit is big enough.
        // In particular, this means we don't do any wrapping if m_lengthLimit=npos.
        if (m_lengthLimit >= me.text.size()) {
            break;
        }

        // Fine already?
        size_t myLength = u8.length(me.text);
        if (myLength <= m_lengthLimit) {
            break;
        }

        // Find break point
        size_t numToKeep = m_lengthLimit, firstToCarry = 0;
        while (numToKeep > 0) {
            Unichar_t ch = u8.charAt(me.text, numToKeep);
            if (isSpace(ch)) {
                // When looking at a space, discard that
                firstToCarry = numToKeep+1;
                break;
            }
            if (numToKeep < m_lengthLimit && isSeparator(ch)) {
                // When looking at a separator, break after it
                ++numToKeep;
                firstToCarry = numToKeep;
                break;
            }
            --numToKeep;
        }

        // If no sensible breakpoint found, break in a word
        if (numToKeep == 0) {
            numToKeep = m_lengthLimit;
            firstToCarry = m_lengthLimit;
        }

        // Build new line
        String_t textToCarry = me.text.substr(u8.charToBytePos(me.text, firstToCarry));
        if (me.hasContinuation && !hasProtectedPrefix(line+1)) {
            // Join with existing line
            Line& next = getLine(line+1);
            Unichar_t lastChar = u8.charAt(textToCarry, u8.length(textToCarry)-1);
            if (!isSpace(lastChar) && !isSeparator(lastChar)) {
                textToCarry += " ";
            }
            next.text.insert(0, textToCarry);
            modifyLine(n, line+1);
        } else {
            // Make new line which will be the end of this paragraph
            insertLine(n, line+1, textToCarry, false);
        }
        me.text.erase(u8.charToBytePos(me.text, numToKeep));
        me.hasContinuation = true;
        modifyLine(n, line);

        // Adjust cursor
        if (m_currentLine == line && m_currentColumn >= firstToCarry) {
            if (m_currentLine < m_maxLine) {
                ++m_currentLine;
                m_currentColumn -= firstToCarry;
            } else {
                m_currentColumn = numToKeep;
            }
        }

        // Next line may still be overlong
        ++line;
    }
}

bool
util::editor::Editor::hasProtectedPrefix(size_t n) const
{
    return (n < m_lines.size()
            && m_lines[n]->protectUntil > 0);
}

bool
util::editor::Editor::isProtectedLine(size_t n) const
{
    // protectUntil is specified as a character count, not byte count.
    // This seemingly-redundant condition tries to avoid computing Utf8().length() for common cases,
    // namely protectUntil=0 (not protected) or protectUntil=huge (protected).
    return (n < m_lines.size()
            && m_lines[n]->protectUntil > 0
            && (m_lines[n]->protectUntil > m_lines[n]->text.size()
                || m_lines[n]->protectUntil > Utf8(0).length(m_lines[n]->text)));
}

size_t
util::editor::Editor::skipProtectedLines(size_t startAt) const
{
    while (isProtectedLine(startAt)) {
        ++startAt;
    }
    return startAt;
}

void
util::editor::Editor::trimLines()
{
    while (m_lines.size() > m_lineLimit) {
        m_lines.popBack();
    }
}

void
util::editor::Editor::limitColumn(Flags_t flags)
{
    if (!flags.contains(AllowCursorAfterEnd)) {
        m_currentColumn = std::min(m_currentColumn, getLineLength(m_currentLine));
    }
}

util::editor::Editor::Line&
util::editor::Editor::getLine(size_t line)
{
    while (m_lines.size() <= line) {
        m_lines.pushBackNew(new Line(0, String_t(), false));
    }
    return *m_lines[line];
}

size_t
util::editor::Editor::getLineLength(size_t line) const
{
    if (line < m_lines.size()) {
        return Utf8(0).length(m_lines[line]->text);
    } else {
        return 0;
    }
}
