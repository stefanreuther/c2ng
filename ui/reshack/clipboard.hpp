/**
  *  \file ui/reshack/clipboard.hpp
  *  \brief Class ui::reshack::Clipboard
  */
#ifndef C2NG_UI_RESHACK_CLIPBOARD_HPP
#define C2NG_UI_RESHACK_CLIPBOARD_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/translator.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/reshack/tool.hpp"

namespace ui { namespace reshack {

    /** Clipboard for reshack.
        The clipboard can store an image.
        The image can have a transparent color. */
    class Clipboard {
     public:
        /** Copy tool.
            Allows user to draw a rectangle, and copies the content into the clipboard. */
        class CopyTool : public Tool {
         public:
            /** Constructor.
                @param parent Clipboard
                @param tx     Translator */
            CopyTool(Clipboard& parent, afl::string::Translator& tx);

            // Tool:
            void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
            void drag(gfx::BaseContext& c, gfx::Point pt);
            void release(gfx::BaseContext& c, gfx::Point pt);
            bool isUsable();

         private:
            Clipboard& m_parent;
            gfx::Point m_prev;
            gfx::Color_t m_backgroundColor;
        };

        /** Paste tool.
            Pastes one (single=true) or multiple (single=false) copies of the clipboard. */
        class PasteTool : public Tool {
         public:
            /** Constructor.
                @param parent Clipboard
                @param single Personality (true: place single copy; false: use clipboard as brush=
                @param tx     Translator */
            PasteTool(Clipboard& parent, bool single, afl::string::Translator& tx);

            // Tool:
            void click(gfx::BaseContext& c, gfx::Point pt, gfx::Color_t bg);
            void drag(gfx::BaseContext& c, gfx::Point pt);
            void release(gfx::BaseContext& c, gfx::Point pt);
            bool isUsable();
         private:
            Clipboard& m_parent;
        };

        /** Constructor.
            Makes an empty clipboard. */
        Clipboard();

        /** Check for content.
            @return true if clipboard has any content */
        bool hasContent() const;

        /** Get content pixmap.
            @return pixmap; null if none */
        afl::base::Ptr<gfx::PalettizedPixmap> getPixmap() const;

        /** Get content color-key.
            @return color that is to be treated as transparent */
        gfx::Color_t getColorKey() const;

        /** Set content.
            @param content    New content pixmap
            @param colorKey   New content color-key */
        void set(afl::base::Ptr<gfx::PalettizedPixmap> content, gfx::Color_t colorKey);

        /** Signal: clipboard has changed */
        afl::base::Signal<void()> sig_change;

     private:
        afl::base::Ptr<gfx::PalettizedPixmap> m_content;
        gfx::Color_t m_colorKey;
    };

} }

#endif
