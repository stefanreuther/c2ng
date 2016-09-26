/**
  *  \file ui/widgets/inputline.cpp
  */

#include "ui/widgets/inputline.hpp"
#include "afl/charset/utf8.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"
#include "ui/colorscheme.hpp"
#include "gfx/complex.hpp"
#include "ui/window.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/widgets/statictext.hpp"
#include "afl/base/deleter.hpp"
#include "ui/group.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/button.hpp"
#include "ui/eventloop.hpp"

namespace {
    const int CursorWidth = 2;

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
}

// /** Input line.
//     \param maxLength   maximum length of input (a hard limit)
//     \param hotkey    hot-key that brings focus to this widget
//     \param aflags  options
//     \param id     widget Id */
ui::widgets::InputLine::InputLine(size_t maxLength, Root& root)
    : SimpleWidget(),
      m_hotkey(0),
      m_maxLength(maxLength),
      m_cursorIndex(0),
      m_firstIndex(0),
      m_text(),
      m_font(),
      m_preferredLength(maxLength < 40 ? maxLength : 40),
      m_root(root),
      m_utf8()
{
    // ex UIInputLine::UIInputLine
    setFlag(TypeErase, true);
}

// /** Input line.
//     \param maxLength   maximum length of input (a hard limit)
//     \param preferredLength preferred width of input, in "em" widths
//     \param hotkey    hot-key that brings focus to this widget
//     \param aflags  options
//     \param id     widget Id */
ui::widgets::InputLine::InputLine(size_t maxLength, int preferredLength, Root& root)
    : SimpleWidget(),
      m_hotkey(0),
      m_maxLength(maxLength),
      m_cursorIndex(0),
      m_firstIndex(0),
      m_text(),
      m_font(),
      m_preferredLength(preferredLength),
      m_root(root),
      m_utf8()
{
    setFlag(TypeErase, true);
}

// /** Set contents of input line. */
ui::widgets::InputLine&
ui::widgets::InputLine::setText(String_t s)
{
    m_text = s;
    m_cursorIndex = m_utf8.length(s);
    m_firstIndex = 0;
    scroll();
    requestRedraw();
    sig_change.raise();
    return *this;
}

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
ui::widgets::InputLine::setFont(gfx::FontRequest& font)
{
    m_font = font;
    return *this;
}

// /** Insert text at current cursor position. Respects flags
//     ilf_NonEditable (=request is ignored) and ilf_TypeErase (=replace
//     current text). */
void
ui::widgets::InputLine::insert(String_t s)
{
    // ex UIInputLine::insert
    if (m_flags.contains(NonEditable)) {
        return;
    }
    if (m_flags.contains(TypeErase)) {
        m_text = s;
        m_cursorIndex = m_utf8.length(s);
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

// /** Move cursor to position pos.
//     \param pos cursor position, [0, lengthUtf8()] */
void
ui::widgets::InputLine::setCursor(size_t pos)
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

// /** Get cursor position. Note that this position is in runes, not bytes! */
size_t
ui::widgets::InputLine::getCursor() const
{
    return m_cursorIndex;
}

// /** Do a standard input line dialog.
//     \param str     string to enter/modify
//     \param maxlen  maximum length for string
//     \param title,req  title and prompt of dialog window
//     \param width   width of input field in "em" widths
//     \param flags   flags for input field */
bool
ui::widgets::InputLine::doStandardDialog(String_t title, String_t prompt)
{
    // ex  UIInputLine::run (totally modified)
//     if (width * font_ems[FONT_TITLE] > ui_root->getExtent().w - 40)
//         width = (ui_root->getExtent().w - 40) / font_ems[FONT_TITLE];
    afl::base::Deleter del;
    EventLoop loop(m_root);

    Window& window = del.addNew(new Window(title, m_root.provider(), BLUE_WINDOW, ui::layout::VBox::instance5));
    // FIXME: border 10

    window.add(del.addNew(new StaticText(prompt, SkinColor::Static, gfx::FontRequest().addSize(1), m_root.provider())));
    window.add(*this);

    // FIXME: UIQuit quit(0);
    Group& buttons = del.addNew(new Group(ui::layout::HBox::instance5));
    buttons.add(del.addNew(new Spacer()));

    Button& okButton = del.addNew(new Button("!OK", util::Key_Return, m_root.provider(), m_root.colorScheme()));
    Button& cancelButton = del.addNew(new Button("!Cancel", util::Key_Escape, m_root.provider(), m_root.colorScheme()));
    okButton.sig_fire.addNewClosure(loop.makeStop(1));
    cancelButton.sig_fire.addNewClosure(loop.makeStop(0));
    buttons.add(okButton);
    buttons.add(cancelButton);
    window.add(buttons);

    // Do it
    window.pack();
    requestFocus();
    m_root.centerWidget(window);
    m_root.addChild(window, 0);
    return loop.run() != 0;
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
                setCursor(m_cursorIndex - 1);
            }
            return true;
         case util::Key_Right:
            /* Move cursor right */
            requestActive();
            setFlag(TypeErase, false);
            if (m_cursorIndex < m_utf8.length(m_text)) {
                setCursor(m_cursorIndex + 1);
            }
            return true;
         case util::KeyMod_Ctrl + util::Key_Left:
            /* Move cursor left one word */
            requestActive();
            setFlag(TypeErase, false);
            setCursor(getPreviousWordBoundary(m_utf8, m_text, m_cursorIndex));
            return true;
         case util::KeyMod_Ctrl + util::Key_Right:
            /* Move cursor right one word */
            requestActive();
            setFlag(TypeErase, false);
            setCursor(getNextWordBoundary(m_utf8, m_text, m_cursorIndex));
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
                requestRedraw();
                sig_change.raise();
            }
            return true;
         case util::Key_Home:
            /* Go to beginning */
            requestActive();
            setFlag(TypeErase, false);
            setCursor(0);
            return true;
         case util::Key_End:
            /* Go to end */
            requestActive();
            setFlag(TypeErase, false);
            setCursor(m_utf8.length(m_text));
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
            setCursor(0);
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
                setCursor(pos);
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
                setCursor(pos+1);
                requestRedraw();
                sig_change.raise();
            }
            return true;
         }

         default:
            if ((key & util::KeyMod_Mask) == 0 && (key < util::Key_FirstSpecial) && acceptUnicode(key)) {
                /* Self-insert */
                String_t n;
                m_utf8.append(n, key);
                requestActive();
                insert(n);
                return true;
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
        requestActive();
        requestFocus();

        size_t new_pos = m_firstIndex;
        int delta = pt.getX() - getExtent().getLeftX();
        afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_font);
        if (font.get() != 0) {
            if (m_flags.contains(Hidden)) {
                while (new_pos < m_utf8.length(m_text) && font->getTextWidth(String_t(new_pos - m_firstIndex, '*')) < delta) {
                    ++new_pos;
                }
            } else {
                while (new_pos < m_utf8.length(m_text) && font->getTextWidth(m_utf8.substr(m_text, m_firstIndex, new_pos - m_firstIndex)) < delta) {
                    ++new_pos;
                }
            }
        }

        // FIXME: consumeprefix()
        m_cursorIndex = new_pos;
        setFlag(TypeErase, false);
        requestRedraw();
        return true;
    } else {
        return false;
    }
}

// Widget:
void
ui::widgets::InputLine::draw(gfx::Canvas& can)
{
    // ex UIInputLine::drawContent
    gfx::Context ctx(can);
    ctx.useColorScheme(m_root.colorScheme());

    gfx::Rectangle r(getExtent());
    int bgcolor;
    if (getFocusState() != NoFocus) {
        scroll();
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
        m_firstIndex = 0;
    }

    String_t t = (m_flags.contains(Hidden)
                  ? String_t(m_utf8.length(m_text) - m_firstIndex, '*')
                  : String_t(m_utf8.substr(m_text, m_firstIndex, String_t::npos)));

    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_font);
    if (font.get() != 0) {
        ctx.useFont(*font);
    }
    drawSolidBar(ctx, r, bgcolor);
    outTextF(ctx, r, t);

    /* outTextF sets cursor to the end of the output */
    drawSolidBar(ctx, gfx::Rectangle(ctx.getCursor().getX(), r.getTopY(), r.getRightX() - ctx.getCursor().getX(), r.getHeight()), Color_Gray);
    if (getFocusState() != NoFocus && m_cursorIndex >= m_firstIndex) {
        int cx = font->getTextWidth(m_utf8.substr(t, 0, m_cursorIndex-m_firstIndex));
        if (cx + 10 < r.getWidth()) {
            /* FIXME: magic numbers */
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX() + cx, r.getTopY() + font->getLineHeight()*9/10, font->getEmWidth()/2, std::max(font->getLineHeight()/10, 1)), Color_Black);
        }
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
    // FIXME: do we need to do more?
    requestRedraw();
}

ui::layout::Info
ui::widgets::InputLine::getLayoutInfo() const
{
    // ex UIInputLine::getLayoutInfo
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_font);
    if (font.get() != 0) {
        return ui::layout::Info(font->getCellSize().scaledBy(4, 1),
                                font->getCellSize().scaledBy(m_preferredLength, 1),
                                ui::layout::Info::GrowHorizontal);
    } else {
        return ui::layout::Info();
    }
}

// /** Adjust display so that cursor is visible. */
void
ui::widgets::InputLine::scroll()
{
    // ex UIInputLine::scroll()
    afl::base::Ptr<gfx::Font> font = m_root.provider().getFont(m_font);
    if (font.get() != 0) {
        const size_t textLength = m_utf8.length(m_text);

        if (m_cursorIndex > textLength) {
            m_cursorIndex = textLength;
        }
        if (m_cursorIndex < 3) {
            m_firstIndex = 0;
        } else if (m_firstIndex > m_cursorIndex - 3) {
            m_firstIndex = m_cursorIndex - 3;
        }

        String_t t = m_flags.contains(Hidden) ? String_t(textLength, '*') : m_text;
        int ln = font->getTextWidth(m_utf8.substr(t, m_firstIndex, m_cursorIndex - m_firstIndex));
        int av = getExtent().getWidth() - CursorWidth;
        if (m_cursorIndex) {
            while (m_firstIndex < m_cursorIndex-1 && ln > av) {
                ++m_firstIndex;
                ln = font->getTextWidth(m_utf8.substr(t, m_firstIndex, m_cursorIndex - m_firstIndex));
            }
        }
    }
}

// /** Check whether an Unicode character should be accepted. */
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
    // FIXME: missing
    // if (hasState(ilf_GameChars) && !isReliableGameCharacter(uni)) {
    //     // Refuse non-game characters if requested
    //     return false;
    // }
    return true;
}
