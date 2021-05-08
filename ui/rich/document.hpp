/**
  *  \file ui/rich/document.hpp
  */
#ifndef C2NG_UI_RICH_DOCUMENT_HPP
#define C2NG_UI_RICH_DOCUMENT_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/icons/icon.hpp"
#include "util/rich/text.hpp"
#include "util/skincolor.hpp"
#include "afl/base/deleter.hpp"

namespace ui { namespace rich {

// /** Rich Document. This class allows formatting of RichText. It breaks
//     lines according to margins, and adjusts line heights according to
//     used fonts.

//     In addition to text, a RichDocument allows usage of block objects,
//     which are instances of RichDocumentBlockObject, and can appear as
//     floating block objects at the left or right side of the document,
//     or as centered block objects interrupting the text flow. The main
//     use for this feature is images.

//     To use, configure the width and margin as needed, and add text
//     using add(), addNewline(), etc., in any order you wish. This will
//     accumulate all the text in an internal buffer of Item's
//     (\c content); partially finished items will remain in an invisible
//     intermediate buffer (\c last_chunk). To finish the document, call
//     finish(). Afterwards, the RichDocument can be used for display. */
    class Document {
     public:
        /* These values are sorted such that one can test '& Link' for link text */
        enum ItemKind {
            Normal     = 0,
            LinkTarget = 1,
            Link       = 0x80,
            LinkHover  = 0x81,
            LinkFocus  = 0x82
        };
        
        struct Item {
            ItemKind kind;
            int x;
            int y;
            int w;           /* display width (0 for LinkTarget). */
            gfx::FontRequest font;
            util::SkinColor::Color color;
            bool underline;
            bool key;
            String_t text;
            bool breakable;

            Item(ItemKind kind, int x, int y, int w, gfx::FontRequest font, util::SkinColor::Color color, bool underline, bool key, const String_t& text, bool breakable)
                : kind(kind), x(x), y(y), w(w),
                  font(font), color(color),
                  underline(underline), key(key), text(text),
                  breakable(breakable)
                { }
        };

        struct BlockItem {
            enum Kind {
                Right,
                Left,
                Center
            };
            Kind kind;
            gfx::Rectangle pos;
            ui::icons::Icon& obj;

            BlockItem(Kind k, ui::icons::Icon& obj);
        };

        /** Link id. To users, this is an opaque value which uniquely identifies a link.
            Internally, it is the index into \c content pointing to the LinkTarget item. */
        typedef size_t LinkId_t;
        static const LinkId_t nil = LinkId_t(-1);

        Document(gfx::ResourceProvider& provider);
        ~Document();

        gfx::ResourceProvider& provider();
        afl::base::Deleter& deleter();

        void clear();
        void setPageWidth(int width);
        void setLeftMargin(int lm);
        void setRightMargin(int rm);
        int getPageWidth() const;
        int getLeftMargin() const;
        int getRightMargin() const;

        void add(const util::rich::Text& text);
        void add(const String_t& text);
        void add(const char* text);
        void addWordSeparator();
        void addNewline();
        void addParagraph();
        void addAt(int x, const util::rich::Text& text);
        void addRight(int x, const util::rich::Text& text);
        void addCentered(int x, const util::rich::Text& text);
        void addPreformatted(const util::rich::Text& text);
        void addFloatObject(ui::icons::Icon& obj, bool left);
        void addCenterObject(ui::icons::Icon& obj);
        void tabTo(int x);
        void finish();
        int getDocumentHeight() const;
        int getDocumentWidth() const;

        void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, int skipY);

        LinkId_t getLinkFromPos(gfx::Point pt) const;
        String_t getLinkTarget(LinkId_t link) const;
        void setLinkKind(LinkId_t link, ItemKind kind);

        LinkId_t getNextLink(LinkId_t id) const;
        LinkId_t getNextLink(LinkId_t id, gfx::Rectangle limit) const;
        LinkId_t getPreviousLink(LinkId_t id) const;
        LinkId_t getPreviousLink(LinkId_t id, gfx::Rectangle limit) const;
        bool isLinkVisible(LinkId_t id, gfx::Rectangle limit) const;

        /* Options */
        // FIXME: rework to SmallSet style
        enum {
            /* If set, paragraph breaks are full lines (default: half) */
            FullLinesBetweenParagraphs = 1
        };
        void setRenderOptions(int opts);
        int getRenderOptions() const;

     private:
        class Splitter;
        friend class Splitter;

        gfx::ResourceProvider& m_provider;

        afl::base::Deleter m_deleter;

        /* Up to first_this_line: completely rendered content.
           After first_this_line: x positions have been assigned, y positions have not been assigned yet */
        afl::container::PtrVector<Item> content;
        /* Contains a partial line with no positions assigned.
           If it contains spaces, it is guaranteed to fit into a
           line after the current x. */
        afl::container::PtrVector<Item> last_chunk;
        /* Contains all block objects. */
        afl::container::PtrVector<BlockItem> block_objs;

        /* Render options */
        int render_options;

        int x;
        int y;
        std::size_t first_this_line;

        int page_width;
        int left_margin;
        int right_margin;

        /* Current block object status. Indexed by Left/Right.
           - bo_index: index of last unprocessed block_objs entry. Can point
           past the end of block_objs if all objects of that type have been
           processed.
           - bo_width: width of current block object placeholder
           - bo_height: remaining height of current block object placeholder.
           Nonzero defines the object as being partially placed. */
        size_t bo_index[2];
        int bo_width[2];
        int bo_height[2];

        void process();
        void addY(int dy);
        void findNextObject(int side);
        void startNextObject(int side);
        void flushLine();
        void flushWord();
        void flushItems(std::size_t n);
    };

} }

#endif
