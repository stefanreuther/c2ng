/**
  *  \file client/widgets/consolecontroller.cpp
  *  \brief Class client::widgets::ConsoleController
  */

#include "client/widgets/consolecontroller.hpp"
#include "client/widgets/consoleview.hpp"

// Constructor.
client::widgets::ConsoleController::ConsoleController(ConsoleView& view)
    : InvisibleWidget(),
      m_view(view),
      m_lines(),
      m_topLine()
{ }

// Add a line of text.
void
client::widgets::ConsoleController::addLine(String_t text, gfx::HorizontalAlignment align, int bold, util::SkinColor::Color color)
{
    size_t numLines = static_cast<size_t>(m_view.getNumLines());
    bool isAtEnd = (m_lines.size() - m_topLine <= numLines);

    m_lines.pushBackNew(new Line(text, align, bold, color));

    // If we were at the end before, scroll to the end
    if (isAtEnd && m_lines.size() > numLines) {
        m_topLine = m_lines.size() - numLines;
    }
    render();
}

// Handle keypress.
bool
client::widgets::ConsoleController::handleKey(util::Key_t key, int prefix)
{
    switch (key) {
     case util::Key_Up:
     case util::Key_WheelUp:
        scrollUp(prefix ? prefix : 1);
        return true;

     case util::Key_Down:
     case util::Key_WheelDown:
        scrollDown(prefix ? prefix : 1);
        return true;

     case util::Key_PgUp:
        scrollUp(m_view.getNumLines()-1);
        return true;

     case util::Key_PgDn:
        scrollDown(m_view.getNumLines()-1);
        return true;

     case util::Key_PgDn + util::KeyMod_Ctrl:
     case util::Key_Home + util::KeyMod_Ctrl:
     case util::Key_Home:
        scrollToUnchecked(0);
        return true;

     case util::Key_PgUp + util::KeyMod_Ctrl:
     case util::Key_End + util::KeyMod_Ctrl:
     case util::Key_End:
        scrollDown(m_lines.size());
        return true;

     default:
        return false;
    }
}

// Render entire widget.
void
client::widgets::ConsoleController::render()
{
    const int numDisplayedLines = m_view.getNumLines();
    const size_t numStoredLines = m_lines.size();
    size_t index = m_topLine;
    for (int i = 0; i < numDisplayedLines; ++i) {
        if (index < numStoredLines) {
            m_view.addLine(i, m_lines[index]->text, m_lines[index]->align, m_lines[index]->bold, m_lines[index]->color);
            ++index;
        } else {
            m_view.addLine(i, String_t(), gfx::LeftAlign, 0, util::SkinColor::Static);
        }
    }

    m_view.setScrollbackIndicator(static_cast<int>(numStoredLines - index));
}

// Scroll up n lines.
void
client::widgets::ConsoleController::scrollUp(size_t n)
{
    if (n < m_topLine) {
        // We can scroll
        scrollToUnchecked(m_topLine - n);
    } else {
        // Top reached
        scrollToUnchecked(0);
    }
}

// Scroll down n lines.
void
client::widgets::ConsoleController::scrollDown(size_t n)
{
    const size_t numDisplayedLines = static_cast<size_t>(m_view.getNumLines());
    const size_t numStoredLines = m_lines.size();
    if (numDisplayedLines >= numStoredLines) {
        // Too few lines, cannot scroll down
        scrollToUnchecked(0);
    } else {
        size_t limit = numStoredLines - numDisplayedLines;
        if (m_topLine >= limit || limit - m_topLine < n) {
            // Bottom reached
            scrollToUnchecked(limit);
        } else {
            // We can scroll
            scrollToUnchecked(m_topLine + n);
        }
    }
}

// Scroll to line, unchecked (callers check).
void
client::widgets::ConsoleController::scrollToUnchecked(size_t n)
{
    if (n != m_topLine) {
        m_topLine = n;
        render();
    }
}
