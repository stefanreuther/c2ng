/**
  *  \file ui/rich/document.hpp
  *  \brief Class ui::rich::Document
  */
#ifndef C2NG_UI_RICH_DOCUMENT_HPP
#define C2NG_UI_RICH_DOCUMENT_HPP

#include "afl/base/deleter.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "gfx/resourceprovider.hpp"
#include "ui/icons/icon.hpp"
#include "util/rich/text.hpp"
#include "util/skincolor.hpp"
#include "afl/bits/smallset.hpp"

namespace ui { namespace rich {

    /** Rich Document.
        This class allows formatting of rich text (util::rich::Text).
        It breaks lines according to margins, and adjusts line heights according to used fonts.

        In addition to text, a Document allows usage of block objects, which are instances of ui::icons::Icon,
        and can appear as floating block objects at the left or right side of the document,
        or as centered block objects interrupting the text flow.
        The main use for this feature is images.

        To use, configure the width and margin as needed, and add text using add(), addNewline(), etc., in any order you wish.
        This will accumulate all the text in an internal buffer of Item's (\c content);
        partially finished items will remain in an invisible intermediate buffer (\c last_chunk).
        To finish the document, call finish().
        Afterwards, the RichDocument can be used for display. */
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

        /** Marker meaning "no link". */
        static const LinkId_t nil = LinkId_t(-1);

        /** Default constructor.
            Makes a blank document.
            @param provider ResourceProvider (for fonts) */
        Document(gfx::ResourceProvider& provider);
        ~Document();

        gfx::ResourceProvider& provider();
        afl::base::Deleter& deleter();

        /** Clear this document.
            Resets everything except for the page width. */
        void clear();

        /** Set this document's page width.
            @param width Page width in pixels */
        void setPageWidth(int width);

        /** Set this document's left margin.
            @param lm Left margin in pixels. */
        void setLeftMargin(int lm);

        /** Set this document's right margin.
            @param rm Right margin in pixels. */
        void setRightMargin(int rm);

        /** Get page width.
            @return page width in pixels */
        int getPageWidth() const;

        /** Get this document's current left margin.
            @return left margin in pixels */
        int getLeftMargin() const;

        /** Get this document's current right margin.
            @return right margin in pixels */
        int getRightMargin() const;

        /** Add rich text.
            @param text Text to add. */
        void add(const util::rich::Text& text);

        /** Add plain text.
            @param text Text to add. */
        void add(const String_t& text);

        /** Add plain text.
            @param text Text to add. */
        void add(const char* text);

        /** Add zero-width word separator.
            Flushes the current word and allows a new line to be begun. */
        void addWordSeparator();

        /** Add newline.
            Think of this like a HTML "br" tag. */
        void addNewline();

        /** Add new paragraph.
            Think of this like a HTML "p" tag. */
        void addParagraph();

        /** Add text at horizontal position.
            The position may be outside the margin.
            Further text will be produced after the just-added text.
            This can be used to produce bullets.
            If the position is before the current output position, this starts a new line.
            @param x Position
            @param text Text to add */
        void addAt(int x, const util::rich::Text& text);

        /** Add right-justified column text.
            Text is aligned such that its right end is at the given x position.
            @param x Position
            @param text Text to add */
        void addRight(int x, const util::rich::Text& text);

        /** Add centered text.
            Text is centered around the given X position.
            @param x Position
            @param text Text to add */
        void addCentered(int x, const util::rich::Text& text);

        /** Add preformatted text.
            Text is written with no word wrap (right margin is temporarily turned off).
            @param text Text to add */
        void addPreformatted(const util::rich::Text& text);

        /** Add floating object.
            The object will be placed at the left or right border.
            Text flows around it.

            @param obj Object. Lifetime must be managed by caller. Caller can use the deleter() to do that.
            @param left true to place object on the left, false to place object on the right */
        void addFloatObject(ui::icons::Icon& obj, bool left);

        /** Add centered object.
            The object will be placed in the center of the page.
            It will interrupt the text flow.

            @param obj Object. Lifetime must be managed by caller. Caller can use the deleter() to do that. */
        void addCenterObject(ui::icons::Icon& obj);

        /** Move to horizontal position.
            Further text will be produced starting here.
            The position may be outside the margin.
            If the position is before the current output position, this starts a new line.
            @param x Position */
        void tabTo(int x);

        /** Finish this document.
            Writes any possible partial line. */
        void finish();

        /** Get height of document.
            @return height in pixels */
        int getDocumentHeight() const;

        /** Get document width.
            Computes the effective width of the document.
            @return width in pixels */
        int getDocumentWidth() const;

        /** Draw document.

            The area/skipY parameters are used to select parts to draw.
            Caller must still clip away partial objects.

            @param ctx Context
            @param area Area to draw in
            @param skipY Skip this many pixels from the top of the document */
        void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, int skipY);

        /** Get link, given a position.
            Locates a link that appears at the specified position and returns its Id,
            an opaque value usable to query information about the link.

            @param pt Point to look up
            @return Link Id for that point, nil if none */
        LinkId_t getLinkFromPos(gfx::Point pt) const;

        /** Get link target for a link.
            @param link Link Id
            @return link target (parameter to RichTextLinkAttribute); empty if link Id is invalid */
        String_t getLinkTarget(LinkId_t link) const;

        /** Change kind (=status) of a link.
            @param link Link Id
            @param kind New link status (must be one of Item::Link, Item::LinkHover, Item::LinkFocus),
                        otherwise the link will be corrupted. */
        void setLinkKind(LinkId_t link, ItemKind kind);

        /** Get next link.
            @param id Link identifier so start at. nil to get the first link.
            @return Link id of found link, nil if none */
        LinkId_t getNextLink(LinkId_t id) const;

        /** Get next link within range.
            @param id Link identifier so start at. nil to get the first link.
            @param limit Relative area to search in
            @return Link id of found link, nil if none */
        LinkId_t getNextLink(LinkId_t id, gfx::Rectangle limit) const;

        /** Get previous link.
            @param id Link identifier so start at. nil to get the last link.
            @return Link id of found link, nil if none */
        LinkId_t getPreviousLink(LinkId_t id) const;

        /** Get previous link within range.
            @param id Link identifier so start at. nil to get the last link.
            @param limit Relative area to search in
            @return Link id of found link, nil if none */
        LinkId_t getPreviousLink(LinkId_t id, gfx::Rectangle limit) const;

        /** Check whether a link is visible.
            @param id Link identifier, non-nil
            @param limit Range to check
            @return true iff at least a part of this link is visible */
        bool isLinkVisible(LinkId_t id, gfx::Rectangle limit) const;

        /** Options */
        enum Flag {
            /** If set, paragraph breaks are full lines (default: half) */
            FullLinesBetweenParagraphs = 1
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Set rendering options.
            @param opts New options */
        void setRenderOptions(Flags_t opts);

        /** Get rendering options.
            @return options */
        Flags_t getRenderOptions() const;

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
        Flags_t m_renderOptions;

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
