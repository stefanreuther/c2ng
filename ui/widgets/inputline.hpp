/**
  *  \file ui/widgets/inputline.hpp
  *  \brief Class ui::widgets::InputLine
  */
#ifndef C2NG_UI_WIDGETS_INPUTLINE_HPP
#define C2NG_UI_WIDGETS_INPUTLINE_HPP

#include "afl/base/signal.hpp"
#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/translator.hpp"
#include "gfx/fontrequest.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/key.hpp"

namespace ui { namespace widgets {

    /** Input line.
        A standard focusable, scrollable text entry field.

        @change The "ilf_Framed" flag has been removed. To frame an input line, wrap it into a FrameGroup. */
    class InputLine : public SimpleWidget {
     public:
        /** Flags with miscellaneous state. */
        enum Flag {
            NumbersOnly, ///< Accept only digits. ex ilf_NumbersOnly
            Hidden,      ///< "Star out" text. ex ilf_Hidden
            NoHi,        ///< Don't accept characters outside US-ASCII range. ex ilf_NoHi
            NonEditable, ///< Do not allow editing (but allow scrolling). ex ilf_NonEditable
            TypeErase,   ///< Typing will clear the input. ex ilf_TypeErase
            GameChars    ///< Accept only characters from game character set. ex ilf_GameChars
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Input line.
            \param maxLength  maximum length of input (number of characters/UTF-8 runes, a hard limit)
            \param root       UI root */
        InputLine(size_t maxLength, Root& root);

        /** Input line.
            \param maxLength       maximum length of input (number of characters/UTF-8 runes, a hard limit)
            \param preferredLength preferred width of input, for layout, in "em" widths
            \param root       UI root */
        InputLine(size_t maxLength, int preferredLength, Root& root);

        ~InputLine();

        /** Set text.
            This will place the cursor at the end, but not by itself set the TypeErase flag.
            \param s Text */
        InputLine& setText(String_t s);

        /** Get text.
            \return current value */
        String_t getText() const;

        /** Set flag.
            \param flag Flag
            \param enable true to set, false to clear
            \return *this */
        InputLine& setFlag(Flag flag, bool enable);

        /** Get current flags.
            \return flags */
        Flags_t getFlags() const;

        /** Set hotkey.
            The hotkey will request focus for this InputLine.
            \param hotkey Key
            \return *this */
        InputLine& setHotkey(util::Key_t hotkey);

        /** Set font.
            This affects layout, so use before starting the dialog.
            \param font Font
            \return *this */
        InputLine& setFont(const gfx::FontRequest& font);

        /** Insert text at current cursor position.
            Respects flags NonEditable (=call is ignored) and TypeErase (=input replaces content),
            as well as the lenght limit.
            \param s Text to insert */
        void insertText(String_t s);

        /** Set cursor position.
            The position is given in characters (UTF-8 runes).
            \param pos New position [0, Utf8().length()] */
        void setCursorIndex(size_t pos);

        /** Get cursor position.
            \return cursor position (in characters/UTF-8 runes) */
        size_t getCursorIndex() const;

        /** Standard dialog.
            \param title  Window title
            \param prompt Prompt to show above input line
            \param tx     Translator
            \return true if confirmed, false if canceled */
        bool doStandardDialog(String_t title, String_t prompt, afl::string::Translator& tx);
        
        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        /** Signal: text changed.
            Called whenever the value returned by getText() changes. */
        afl::base::Signal<void()> sig_change;

        /** Signal: activate.
            Called whenever the input line is activated by
            - pressing the hot-key
            - pressing Space on a non-editable input
            - clicking it */
        afl::base::Signal<void()> sig_activate;

     private:
        /** Hotkey to focus this widget. */
        util::Key_t m_hotkey;

        /** Maximum length in characters (hard limit). */
        size_t m_maxLength;

        /** Cursor position in characters. */
        size_t m_cursorIndex;

        /** First displayed character position. */
        //size_t m_firstIndex;
        int m_pixelOffset;

        /** Text. */
        String_t m_text;

        /** Font selection. */
        gfx::FontRequest m_font;

        /** Preferred length of this widget, in ems. */
        int m_preferredLength;

        /** Static/dynamic flags. */
        Flags_t m_flags;

        /** True if mouse button is down. */
        bool m_mouseDown;

        /** User-interface root. */
        Root& m_root;

        /** UTF-8 handler (for convenience). */
        afl::charset::Utf8 m_utf8;

        void scroll();
        int getTextWidth() const;
        bool acceptUnicode(uint32_t uni) const;

        String_t getPerceivedText() const;
    };

} }

#endif
