/**
  *  \file ui/widgets/inputline.hpp
  */
#ifndef C2NG_UI_WIDGETS_INPUTLINE_HPP
#define C2NG_UI_WIDGETS_INPUTLINE_HPP

#include "afl/base/types.hpp"
#include "util/key.hpp"
#include "gfx/fontrequest.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/signal.hpp"
#include "ui/simplewidget.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/root.hpp"
#include "afl/charset/utf8.hpp"

namespace ui { namespace widgets {

    // /** \class UIInputLine
    //     \brief Input Line

    //     Standard text entry field */
    class InputLine : public SimpleWidget {
     public:
        // Removed flags:
        // - Framed (use separate frame widget)
        // FIXME: replace NumbersOnly, NoHi, GameChars by a character-acceptor interface
        enum Flag {
            NumbersOnly, //        ilf_NumbersOnly = 2*st_Last,    // accept only digits
            Hidden,      //        ilf_Hidden      = 8*st_Last,    // "star out" text
            NoHi,        //        ilf_NoHi        = 16*st_Last,   // don't accept high-ascii chars
            NonEditable, //        ilf_NonEditable = 32*st_Last,   // non-editable
            TypeErase,   //        ilf_TypeErase   = 64*st_Last,
            GameChars    //        ilf_GameChars   = 128*st_Last   // only characters from game charset
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        InputLine(size_t maxLength, Root& root);
        InputLine(size_t maxLength, int preferredLength, Root& root);

        InputLine& setText(String_t s);
        String_t getText() const;

        InputLine& setFlag(Flag flag, bool enable);
        Flags_t getFlags() const;

        InputLine& setHotkey(util::Key_t hotkey);
        InputLine& setFont(gfx::FontRequest& font);

        void insert(String_t s); // FIXME name
        void setCursor(size_t pos); // FIXME name
        size_t getCursor() const; // FIXME name

        bool doStandardDialog(String_t title, String_t prompt);
        
        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        afl::base::Signal<void()> sig_change;

     private:
        /** Hotkey to focus this widget. */
        util::Key_t m_hotkey;

        /** Maximum length in characters (hard limit). */
        size_t m_maxLength;

        /** Cursor position in characters. */
        size_t m_cursorIndex;

        /** First displayed character position. */
        size_t m_firstIndex;

        /** Text. */
        String_t m_text;

        /** Font selection. */
        gfx::FontRequest m_font;

        /** Preferred length of this widget, in ems. */
        int m_preferredLength;

        /** Static/dynamic flags. */
        Flags_t m_flags;

        /** User-interface root. */
        Root& m_root;

        /** UTF-8 handler (for convenience). */
        afl::charset::Utf8 m_utf8;

        void scroll();
        int getTextWidth() const;
        bool acceptUnicode(uint32_t uni) const;


 //    static bool run(string_t& str, string_t::size_type m_maxLength,
 //                    const string_t& title, const string_t& req,
 //                    int width, int flags);
    };

} }

#endif
