/**
  *  \file client/widgets/turnlistbox.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_TURNLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_TURNLISTBOX_HPP

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "gfx/font.hpp"
#include "gfx/point.hpp"
#include "ui/root.hpp"
#include "ui/widgets/abstractlistbox.hpp"

namespace client { namespace widgets {

    class TurnListbox : public ui::widgets::AbstractListbox {
     public:
        enum Status {
            Unknown,            // I don't know
            Unavailable,        // I know it is not available
            StronglyAvailable,  // I'm certain it's available
            WeaklyAvailable,    // I guess it's available
            Failed,             // Loading failed
            Loaded,             // It is loaded
            Current,            // This is the current turn
            Active              // Loaded and active
        };
        struct Item {
            int turnNumber;
            String_t time;
            Status status;
            Item(int turnNumber, String_t time, Status status)
                : turnNumber(turnNumber), time(time), status(status)
                { }
        };
        typedef std::vector<Item> Items_t;

        TurnListbox(gfx::Point cells, ui::Root& root, afl::string::Translator& tx);

        // AbstractListbox virtuals:
        virtual size_t getNumItems() const;
        virtual bool isItemAccessible(size_t n) const;
        virtual int getItemHeight(size_t n) const;
        virtual int getHeaderHeight() const;
        virtual int getFooterHeight() const;
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawFooter(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);

        void swapItems(Items_t& items);
        void setItem(size_t index, const Item& content);
        const Item* getItem(size_t n);

     private:
        Items_t m_items;
        gfx::Point m_cells;
        ui::Root& m_root;
        afl::string::Translator& m_translator;
        afl::base::Ref<gfx::Font> m_bigFont;
        afl::base::Ref<gfx::Font> m_smallFont;
    };

} }

#endif
