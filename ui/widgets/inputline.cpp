/**
  *  \file ui/widgets/inputline.cpp
  *  \brief Class ui::widgets::InputLine
  */

#include "ui/widgets/inputline.hpp"
#include "afl/base/deleter.hpp"
#include "afl/charset/utf8.hpp"
#include "gfx/clipfilter.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "ui/colorscheme.hpp"
#include "ui/draw.hpp"
#include "ui/eventloop.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/widgets/quit.hpp"
#include "ui/widgets/statictext.hpp"
#include "ui/window.hpp"
#include "util/unicodechars.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"

namespace {
    const char*const DEFAULT_FONT = "+";

    size_t getPreviousWordBoundary(afl::charset::Utf8& u8, const String_t& str, size_t pos)
    {
        while (pos > 0 && u8.charAt(str, pos-1) == ' ')
            --pos;
        while (pos > 0 && u8.charAt(str, pos-1) != ' ')
            --pos;
        return pos;
    }

    size_t getNextWordBoundary(afl::charset::Utf8& u8, const String_t& str, size_t pos)
    {
        const size_t max = u8.length(str);
        while (pos < max && u8.charAt(str, pos) == ' ')
            ++pos;
        while (pos < max && u8.charAt(str, pos) != ' ')
            ++pos;
        return pos;
    }

    int getCursorWidth(gfx::Font& font)
    {
        return font.getEmWidth() / 2;
    }
}

ui::widgets::InputLine::InputLine(size_t maxLength, Root& root)
    : SimpleWidget(),
      m_hotkey(0),
      m_maxLength(maxLength),
      m_cursorIndex(0),
      m_pixelOffset(0),
      m_text(),
      m_font(DEFAULT_FONT),
      m_preferredLength(maxLength < 40 ? int(maxLength) : 40),
      m_flags(),
      m_mouseDown(false),
      m_root(root),
      m_utf8()
{
    // ex UIInputLine::UIInputLine
    setFlag(TypeErase, true);
}

ui::widgets::InputLine::InputLine(size_t maxLength, int preferredLength, Root& root)
    : SimpleWidget(),
      m_hotkey(0),
      m_maxLength(maxLength),
      m_cursorIndex(0),
      m_pixelOffset(0),
      m_text(),
      m_font(DEFAULT_FONT),
      m_preferredLength(preferredLength),
      m_flags(),
      m_mouseDown(false),
      m_root(root),
      m_utf8()
{
    setFlag(TypeErase, true);
}

ui::widgets::InputLine&
ui::widgets::InputLine::setText(String_t s)
{
    m_text = s;
    m_cursorIndex = m_utf8.length(s);
    m_pixelOffset = 0;
    scroll();
    requestRedraw();
    sig_change.raise();
    return *this;
}

ui::widgets::InputLine::~InputLine()
{ }

String_t
ui::widgets::InputLine::getText() const
{
    return m_text;
}

ui::widgets::InputLine&
ui::widgets::InputLine::setFlag(Flag flag, bool enable)
{
    Flags_t old = m_flags;
    if (enable) {
        m_flags += flag;
    } else {
        m_flags -= flag;
    }
    if (m_flags != old) {
        if (flag == TypeErase) {
            requestRedraw();
        }
    }
    return *this;
}

ui::widgets::InputLine::Flags_t
ui::widgets::InputLine::getFlags() const
{
    return m_flags;
}

ui::widgets::InputLine&
ui::widgets::InputLine::setHotkey(util::Key_t hotkey)
{
    m_hotkey = hotkey;
    return *this;
}

ui::widgets::InputLine&
ui::widgets::InputLine::setFont(const gfx::FontRequest& font)
{
    m_font = font;
    return *this;
}

void
ui::widgets::InputLine::insertText(String_t s)
{
    // ex UIInputLine::insert
    if (m_flags.contains(NonEditable)) {
        return;
    }
    if (m_flags.contains(TypeErase)) {
        // Replace
        m_text = m_utf8.substr(s, 0, m_maxLength);
        m_cursorIndex = m_utf8.length(m_text);
        setFlag(TypeErase, false);
    } else {
        // Can we insert?
        size_t textLength = m_utf8.length(m_text);
        if (textLength >= m_maxLength)
            return;

        // Limit inserted text
        s = m_utf8.substr(s, 0, m_maxLength - textLength);

        // Compute new text
        m_text = m_utf8.substr(m_text, 0, m_cursorIndex) + s + m_utf8.substr(m_text, m_cursorIndex, String_t::npos);
        m_cursorIndex += m_utf8.length(s);
    }
    scroll();
    requestRedraw();
    sig_change.raise();
}

void
ui::widgets::InputLine::setCursorIndex(size_t pos)
{
    // ex UIInputLine::setCursor
    const size_t textLength = m_utf8.length(m_text);
    if (pos > textLength) {
        pos = textLength;
    }
    if (pos != m_cursorIndex) {
        setFlag(TypeErase, false);
        m_cursorIndex = pos;
        scroll();
        requestRedraw();
    }
}

size_t
ui::widgets::InputLine::getCursorIndex() const
{
    return m_cursorIndex;
}

bool
ui::widgets::InputLine::doStandardDialog(String_t title, String_t prompt, afl::string::Translator& tx)
{
    // ex UIInputLine::run (totally modified)
    return ui::widgets::doStandardDialog(title, prompt, *this, false, m_root, tx);
}

// EventConsumer:
bool
ui::widgets::InputLine::handleKey(util::Key_t key, int /*prefix*/)
{
    if (hasState(DisabledState)) {
        // Ignore
    } else if (!hasState(FocusedState)) {
        if (key == m_hotkey) {
            requestActive();
            requestFocus();
            sig_activate.raise();
            return true;
        }
    } else {
        size_t pos;
        switch(key) {
         case util::Key_Left:
            /* Move cursor left */
            requestActive();
            setFlag(TypeErase, false);
            if (m_cursorIndex > 0) {
                setCursorIndex(m_cursorIndex - 1);
            }
            return true;
         case util::Key_Right:
            /* Move cursor right */
            requestActive();
            setFlag(TypeErase, false);
            if (m_cursorIndex < m_utf8.length(m_text)) {
                setCursorIndex(m_cursorIndex + 1);
            }
            return true;
         case util::KeyMod_Ctrl + util::Key_Left:
            /* Move cursor left one word */
            requestActive();
            setFlag(TypeErase, false);
            setCursorIndex(getPreviousWordBoundary(m_utf8, m_text, m_cursorIndex));
            return true;
         case util::KeyMod_Ctrl + util::Key_Right:
            /* Move cursor right one word */
            requestActive();
            setFlag(TypeErase, false);
            setCursorIndex(getNextWordBoundary(m_utf8, m_text, m_cursorIndex));
            return true;
         case util::Key_Backspace:
            /* Delete character backward */
            requestActive();
            setFlag(TypeErase, false);
            if (m_cursorIndex > 0) {
                --m_cursorIndex;
                if (!m_flags.contains(NonEditable)) {
                    m_text = m_utf8.substr(m_text, 0, m_cursorIndex) + m_utf8.substr(m_text, m_cursorIndex+1, String_t::npos);
                    sig_change.raise();
                }
                requestRedraw();
            }
            return true;
         case util::Key_Delete:
         case util::KeyMod_Ctrl + 'd':
            /* Delete character forward, or delete whole text */
            /* Note that in PCC 1.x, Ctrl-D is delete word forward */
            requestActive();
            if (!m_flags.contains(NonEditable)) {
                if (m_flags.contains(TypeErase)) {
                    m_text.erase();
                    setFlag(TypeErase, false);
                } else {
                    if (m_cursorIndex < m_utf8.length(m_text)) {
                        m_text = m_utf8.substr(m_text, 0, m_cursorIndex) + m_utf8.substr(m_text, m_cursorIndex+1, String_t::npos);
                    }
                }
                scroll();
                requestRedraw();
                sig_change.raise();
            }
            return true;
         case util::Key_Home:
            /* Go to beginning */
            requestActive();
            setFlag(TypeErase, false);
            setCursorIndex(0);
            return true;
         case util::Key_End:
            /* Go to end */
            requestActive();
            setFlag(TypeErase, false);
            setCursorIndex(m_utf8.length(m_text));
            return true;
         case util::KeyMod_Ctrl + 'y':
            /* Delete whole line */
            requestActive();
            setFlag(TypeErase, false);
            if (!m_flags.contains(NonEditable)) {
                m_text.erase();
                requestRedraw();
                sig_change.raise();
            }
            setCursorIndex(0);
            return true;
         case util::KeyMod_Ctrl + 'k':
            /* Delete till end of line */
            requestActive();
            setFlag(TypeErase, false);
            if (!m_flags.contains(NonEditable)) {
                m_text = m_utf8.substr(m_text, 0, m_cursorIndex);
                requestRedraw();
                sig_change.raise();
            }
            return true;
         case util::KeyMod_Ctrl + util::Key_Backspace:
            /* Delete word backward */
            requestActive();
            setFlag(TypeErase, false);
            if (!m_flags.contains(NonEditable)) {
                pos = getPreviousWordBoundary(m_utf8, m_text, m_cursorIndex);
                m_text = m_utf8.substr(m_text, 0, pos) + m_utf8.substr(m_text, m_cursorIndex, String_t::npos);
                setCursorIndex(pos);
                requestRedraw();
                sig_change.raise();
            }
            return true;
         case util::KeyMod_Ctrl + 'u':
            /* No-op, just clear type-erase */
            requestActive();
            setFlag(TypeErase, false);
            return true;
         case util::KeyMod_Ctrl + 't': {
            /* Transpose characters */
            size_t n = m_utf8.length(m_text);
            requestActive();
            setFlag(TypeErase, false);
            if (n >= 2 && !m_flags.contains(NonEditable)) {
                pos = m_cursorIndex;
                if (pos >= n) {
                    pos = n - 1;
                }
                m_text = m_utf8.substr(m_text, 0, pos-1) + m_utf8.substr(m_text, pos, 1) + m_utf8.substr(m_text, pos-1, 1) + m_utf8.substr(m_text, pos+1, String_t::npos);
                setCursorIndex(pos+1);
                requestRedraw();
                sig_change.raise();
            }
            return true;
         }

         default:
            if (m_flags.contains(NonEditable)) {
                if (key == ' ') {
                    requestActive();
                    sig_activate.raise();
                    return true;
                }
            } else {
                if ((key & util::KeyMod_Mask) == 0 && (key < util::Key_FirstSpecial) && acceptUnicode(key)) {
                    /* Self-insert */
                    String_t n;
                    m_utf8.append(n, key);
                    requestActive();
                    insertText(n);
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool
ui::widgets::InputLine::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (pressedButtons.nonempty() && getExtent().contains(pt) && !hasState(DisabledState)) {
        // Mouse pressed in widget: activate and place cursor.
        // Cursor follows mouse while button is pressed.
        requestActive();
        requestFocus();

        m_root.consumeMousePrefixArgument();
        m_mouseDown = true;

        // Find new position
        afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
        const String_t text = getPerceivedText();
        int pixelPos = m_pixelOffset + pt.getX() - getExtent().getLeftX();
        size_t charPos = 0;
        size_t length = m_utf8.length(text);
        while (charPos <= length && font->getTextWidth(m_utf8.substr(m_text, 0, charPos)) < pixelPos) {
            ++charPos;
        }
        if (charPos > 0) {
            --charPos;
        }
        m_cursorIndex = charPos;
        setFlag(TypeErase, false);
        requestRedraw();
        return true;
    } else {
        // Mouse not pressed in widget.
        // If it previously was pressed, and the widget is still focused, it was clicked.
        // (Otherwise, it was dragged into another widget.)
        bool down = m_mouseDown;
        m_mouseDown = false;
        if (pressedButtons.empty() && down && hasState(FocusedState) && hasState(ActiveState)) {
            sig_activate.raise();
        }
        return false;
    }
}

// Widget:
void
ui::widgets::InputLine::draw(gfx::Canvas& can)
{
    // ex UIInputLine::drawContent
    gfx::ClipFilter filter(can, getExtent());
    gfx::Context<uint8_t> ctx(filter, m_root.colorScheme());

    gfx::Rectangle r(getExtent());
    uint8_t bgcolor;
    if (getFocusState() != NoFocus) {
        if (m_flags.contains(TypeErase)) {
            ctx.setColor(Color_Gray);
            bgcolor = Color_Blue;
        } else {
            ctx.setColor(Color_Blue);
            bgcolor = Color_Gray;
        }
    } else {
        if (hasState(DisabledState)) {
            ctx.setColor(Color_Dark);
        } else {
            ctx.setColor(Color_Black);
        }
        bgcolor = Color_Gray;
    }

    String_t t = getPerceivedText();

    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    ctx.useFont(*font);

    drawSolidBar(ctx, r, bgcolor);
    outText(ctx, gfx::Point(r.getLeftX() - m_pixelOffset, r.getTopY()), t);

    const int endX = r.getLeftX() - m_pixelOffset + font->getTextWidth(t);
    if (endX < r.getRightX()) {
        drawSolidBar(ctx, gfx::Rectangle(endX, r.getTopY(), r.getRightX() - endX, r.getHeight()), Color_Gray);
    }
    if (getFocusState() != NoFocus) {
        const int cx = font->getTextWidth(m_utf8.substr(t, 0, m_cursorIndex));
        drawSolidBar(ctx, gfx::Rectangle(r.getLeftX() - m_pixelOffset + cx,
                                         r.getTopY() + font->getLineHeight()*9/10,
                                         getCursorWidth(*font),
                                         std::max(font->getLineHeight()/10, 1)),
                     Color_Black);
    }
}

void
ui::widgets::InputLine::handleStateChange(State st, bool enable)
{
    if (st == FocusedState && enable) {
        setFlag(TypeErase, true);
    }
    if (st == FocusedState || st == DisabledState) {
        requestRedraw();
    }
}

void
ui::widgets::InputLine::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    scroll();
    requestRedraw();
}

ui::layout::Info
ui::widgets::InputLine::getLayoutInfo() const
{
    // ex UIInputLine::getLayoutInfo
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    return ui::layout::Info(font->getCellSize().scaledBy(4, 1),
                            font->getCellSize().scaledBy(m_preferredLength, 1),
                            ui::layout::Info::GrowHorizontal);
}

/** Adjust display so that cursor is visible. */
void
ui::widgets::InputLine::scroll()
{
    // ex UIInputLine::scroll()
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(m_font);
    String_t perceivedText = getPerceivedText();

    int cursorWidth  = getCursorWidth(*font);
    int textSize     = font->getTextWidth(perceivedText) + cursorWidth;
    int beforeCursor = font->getTextWidth(m_utf8.substr(perceivedText, 0, m_cursorIndex));
    int width = getExtent().getWidth();

    if (width + m_pixelOffset > textSize) {
        m_pixelOffset = std::max(0, textSize - width);
    } else if (m_pixelOffset >= beforeCursor) {
        m_pixelOffset = beforeCursor;
    } else if (beforeCursor - m_pixelOffset > width - cursorWidth) {
        m_pixelOffset = beforeCursor - width + cursorWidth;
    }
}

/** Check whether an Unicode character should be accepted. */
bool
ui::widgets::InputLine::acceptUnicode(uint32_t uni) const
{
    // ex UIInputLine::acceptUnicode
    if (uni < 32 || uni == 127) {
        // Refuse controls
        return false;
    }
    if (m_flags.contains(NumbersOnly) && (uni < '0' || uni > '9')) {
        // Refuse nondigits if requested
        return false;
    }
    if (m_flags.contains(NoHi) && (uni >= 128)) {
        // Refuse non-ASCII if requested
        return false;
    }
    // FIXME: handle GameChars
    return true;
}

String_t
ui::widgets::InputLine::getPerceivedText() const
{
    if (m_flags.contains(Hidden)) {
        String_t result;
        for (size_t i = 0, n = m_utf8.length(m_text); i < n; ++i) {
            result += UTF_BULLET;
        }
        return result;
    } else {
        return m_text;
    }
}
