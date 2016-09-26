/**
  *  \file client/widgets/turnlistbox.hpp
  */
#ifndef C2NG_CLIENT_WIDGETS_TURNLISTBOX_HPP
#define C2NG_CLIENT_WIDGETS_TURNLISTBOX_HPP

#include "ui/widgets/abstractlistbox.hpp"
#include "afl/string/string.hpp"
#include "ui/root.hpp"
#include "gfx/point.hpp"
#include "gfx/font.hpp"

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

        TurnListbox(gfx::Point cells, ui::Root& root);

        // AbstractListbox virtuals:
        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);

        // Widget:
        virtual ui::layout::Info getLayoutInfo() const;
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);

        void swapItems(Items_t& items);
        void setItem(size_t index, const Item& content);
        const Item* getItem(size_t n);

     private:
        Items_t m_items;
        gfx::Point m_cells;
        ui::Root& m_root;
        afl::base::Ptr<gfx::Font> m_bigFont;
        afl::base::Ptr<gfx::Font> m_smallFont;
    };

} }

#endif
